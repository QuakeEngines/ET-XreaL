#ifndef NODE_H_
#define NODE_H_

// Forward declaration to avoid including the whole libxml2 headers
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

#include <string>
#include <vector>

namespace xml
{

// Typedefs

class Node;
typedef std::vector<Node> NodeList;

/* Node
 * 
 * A representation of an XML node. This class wraps an xmlNodePtr as used
 * by libxml2, and provides certain methods to access properties of the node.
 */

class Node
{
private:

    // The contained xmlNodePtr. This points to part of a wider xmlDoc
    // structure which is not owned by this Node object.
    xmlNodePtr _xmlNode;

public:

    // Construct a Node from the provided xmlNodePtr.
	Node(xmlNodePtr node);
	
	// Get the actual node pointer to a given node
    xmlNodePtr getNodePtr() const;
    
    // Get the name of the given node
    const std::string getName() const;
	
	// Get a list of nodes which are children of this node
    NodeList getChildren() const;

	// Creates a new child under this XML Node
	Node createChild(const std::string& name);

	// Get a list of nodes which are children of this node and match the
    // given name.
    NodeList getNamedChildren(const std::string& name) const;
    
    // Return the value of the given attribute, or an empty string
    // if the attribute is not present on this Node.
    std::string getAttributeValue(const std::string& key) const;
    
    // Set the value of the given attribute    
    void setAttributeValue(const std::string& key, const std::string& value);
    
    /** Return the text content of this node.
     * 
     * @returns
     * The text content of this node.
     */
     
	std::string getContent() const;

	void addText(const std::string& text);

	// Unlink and delete the node and all its children
	void erase();
};


} // namespace xml

#endif /*NODE_H_*/
