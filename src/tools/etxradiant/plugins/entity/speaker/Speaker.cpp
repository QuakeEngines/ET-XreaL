#include "Speaker.h"

#include "itextstream.h"
#include "iregistry.h"
#include "irenderable.h"
#include "isound.h"
#include <stdlib.h>
#include "SpeakerNode.h"
#include "../EntitySettings.h"
#include <boost/bind.hpp>

namespace entity {

	namespace 
	{
		const std::string KEY_S_MAXDISTANCE("s_maxdistance");
		const std::string KEY_S_MINDISTANCE("s_mindistance");
		const std::string KEY_S_SHADER("noise");
	}

Speaker::Speaker(SpeakerNode& node, 
		const Callback& transformChanged, 
		const Callback& boundsChanged) :
	_owner(node),
	m_entity(node._entity),
	m_originKey(boost::bind(&Speaker::originChanged, this)),
	m_origin(ORIGINKEY_IDENTITY),
	_renderableRadii(m_origin, _radiiTransformed),
	m_useSpeakerRadii(true),
	m_minIsSet(false),
	m_maxIsSet(false),
	m_aabb_solid(m_aabb_local),
	m_aabb_wire(m_aabb_local),
	m_transformChanged(transformChanged),
	m_boundsChanged(boundsChanged)
{}

Speaker::Speaker(const Speaker& other, 
		SpeakerNode& node, 
		const Callback& transformChanged, 
		const Callback& boundsChanged) :
	_owner(node),
	m_entity(node._entity),
	m_originKey(boost::bind(&Speaker::originChanged, this)),
	m_origin(ORIGINKEY_IDENTITY),
	_renderableRadii(m_origin, _radiiTransformed),
	m_useSpeakerRadii(true),
	m_minIsSet(false),
	m_maxIsSet(false),
	m_aabb_solid(m_aabb_local),
	m_aabb_wire(m_aabb_local),
	m_transformChanged(transformChanged),
	m_boundsChanged(boundsChanged)
{}

Speaker::~Speaker()
{
	destroy();
}

const AABB& Speaker::localAABB() const {
	return m_aabb_border;
}

// Submit renderables for solid mode
void Speaker::renderSolid(RenderableCollector& collector, 
                          const VolumeTest& volume,
                          const Matrix4& localToWorld,
                          bool isSelected) const
{
	collector.SetState(m_entity.getEntityClass()->getFillShader(), RenderableCollector::eFullMaterials);
	collector.addRenderable(m_aabb_solid, localToWorld);

    // Submit the speaker radius if we are selected or the "show all speaker
    // radii" option is set
	if (isSelected || EntitySettings::InstancePtr()->showAllSpeakerRadii())
    {
		collector.addRenderable(_renderableRadii, localToWorld);
    }
}

// Submit renderables for wireframe mode
void Speaker::renderWireframe(RenderableCollector& collector, 
                              const VolumeTest& volume,
                              const Matrix4& localToWorld,
                              bool isSelected) const
{
	collector.SetState(m_entity.getEntityClass()->getWireShader(), RenderableCollector::eWireframeOnly);
	collector.addRenderable(m_aabb_wire, localToWorld);

    // Submit the speaker radius if we are selected or the "show all speaker
    // radii" option is set
	if (isSelected || EntitySettings::InstancePtr()->showAllSpeakerRadii())
    {
		collector.addRenderable(_renderableRadii, localToWorld);
    }
}

void Speaker::testSelect(Selector& selector, 
	SelectionTest& test, const Matrix4& localToWorld)
{
	test.BeginMesh(localToWorld);

	SelectionIntersection best;
	aabb_testselect(m_aabb_local, test, best);
	if(best.valid()) {
		selector.addIntersection(best);
	}
}

void Speaker::translate(const Vector3& translation)
{
	m_origin += translation;
}

void Speaker::rotate(const Quaternion& rotation)
{
	// nothing to rotate here, speakers are symmetric
}

void Speaker::snapto(float snap) {
	m_originKey.m_origin = origin_snapped(m_originKey.m_origin, snap);
	m_originKey.write(&m_entity);
}

void Speaker::revertTransform()
{
	m_origin = m_originKey.m_origin;

	_radiiTransformed = _radii;
}

void Speaker::freezeTransform() {
	m_originKey.m_origin = m_origin;
	m_originKey.write(&m_entity);

	_radii = _radiiTransformed;

/*
	// Write the s_mindistance/s_maxdistance keyvalues if we have a valid shader
	if (!m_entity.getKeyValue(KEY_S_SHADER).empty())
	{
		// Note: Write the spawnargs in meters

		if (_radii.getMax() != _defaultRadii.getMax())
		{
			m_entity.setKeyValue(KEY_S_MAXDISTANCE, floatToStr(_radii.getMax(true)));
		}
		else
		{
			// Radius is matching default, clear the spawnarg
			m_entity.setKeyValue(KEY_S_MAXDISTANCE, "");
		}

		if (_radii.getMin() != _defaultRadii.getMin())
		{
			m_entity.setKeyValue(KEY_S_MINDISTANCE, floatToStr(_radii.getMin(true)));
		}
		else
		{
			// Radius is matching default, clear the spawnarg
			m_entity.setKeyValue(KEY_S_MINDISTANCE, "");
		}
	}
*/
}

void Speaker::setRadiusFromAABB(const AABB& aabb)
{
	// Find out which dimension got changed the most
	Vector3 delta = aabb.getExtents() - localAABB().getExtents();

	double maxTrans;

	// Get the maximum translation component
	if (fabs(delta.x()) > fabs(delta.y()))
	{
		maxTrans = (fabs(delta.x()) > fabs(delta.z())) ? delta.x() : delta.z();
	}
	else
	{
		maxTrans = (fabs(delta.y()) > fabs(delta.z())) ? delta.y() : delta.z();
	}

	if (EntitySettings::InstancePtr()->dragResizeEntitiesSymmetrically())
	{
		// For a symmetric AABB change, take the extents delta times 2
		maxTrans *= 2;
	}
	else
	{
		// Update the origin accordingly
		m_origin += aabb.origin - localAABB().getOrigin();
	}

	float oldRadius = _radii.getMax() > 0 ? _radii.getMax() : _radii.getMin();

	// Prevent division by zero
	if (oldRadius == 0)
	{
		oldRadius = 1;
	}

	float newMax = static_cast<float>(oldRadius + maxTrans);

	float ratio = newMax / oldRadius;
	float newMin = _radii.getMin() * ratio;

	if (newMax < 0) newMax = 0.02f;
	if (newMin < 0) newMin = 0.01f;

	// Resize the radii and update the min radius proportionally
	_radiiTransformed.setMax(newMax);
	_radiiTransformed.setMin(newMin);

	updateAABB();
	updateTransform();
}

void Speaker::construct()
{
	m_aabb_local = m_entity.getEntityClass()->getBounds();
	m_aabb_border = m_aabb_local;
	
	_owner.addKeyObserver("origin", m_originKey);
	_owner.addKeyObserver(KEY_S_SHADER, _shaderObserver);
	_owner.addKeyObserver(KEY_S_MINDISTANCE, _radiusMinObserver);
	_owner.addKeyObserver(KEY_S_MAXDISTANCE, _radiusMaxObserver);
}

void Speaker::destroy()
{
	_owner.removeKeyObserver("origin", m_originKey);
	_owner.removeKeyObserver(KEY_S_SHADER, _shaderObserver);
	_owner.removeKeyObserver(KEY_S_MINDISTANCE, _radiusMinObserver);
	_owner.removeKeyObserver(KEY_S_MAXDISTANCE, _radiusMaxObserver);
}

void Speaker::updateAABB()
{
	// set the AABB to the biggest AABB the speaker contains
	m_aabb_border = m_aabb_local;

	float radius = _radiiTransformed.getMax();
	m_aabb_border.extents = Vector3(radius, radius, radius);

	m_boundsChanged();
}

void Speaker::updateTransform()
{
	_owner.localToParent() = Matrix4::getTranslation(m_origin);
	m_transformChanged();
}

void Speaker::originChanged()
{
	m_origin = m_originKey.m_origin;
	updateTransform();
}


void Speaker::sSoundChanged(const std::string& value)
{

	// Tr3B: FIXME
	/*
	if (value.empty()) {
		m_stdVal.setMin(0);
		m_stdVal.setMax(0);
	}

	else {
		m_stdVal = GlobalSoundManager().getSoundFile(value).getRadii();
	}
	if (!m_minIsSet) m_speakerRadii.m_radii.setMin(m_stdVal.getMin());
	if (!m_maxIsSet) m_speakerRadii.m_radii.setMax(m_stdVal.getMax());
	*/

	// Store the new values into our working set
	_radiiTransformed = _radii;

	updateAABB();
}

void Speaker::sMinChanged(const std::string& value)
{
	// Check whether the spawnarg got set or removed
	m_minIsSet = value.empty() ? false : true;

	if (m_minIsSet)
	{
		// we need to parse in metres
		_radii.setMin(strToFloat(value), true);
	}
	else 
	{
		_radii.setMin(_defaultRadii.getMin());
	}

	// Store the new value into our working set
	_radiiTransformed.setMin(_radii.getMin());

	updateAABB();
}

void Speaker::sMaxChanged(const std::string& value)
{
	m_maxIsSet = value.empty() ? false : true;

	if (m_maxIsSet)
	{
		// we need to parse in metres
		_radii.setMax(strToFloat(value), true);
	}
	else 
	{
		_radii.setMax(_defaultRadii.getMax());
	}

	// Store the new value into our working set
	_radiiTransformed.setMax(_radii.getMax());

	updateAABB();
}

} // namespace entity
