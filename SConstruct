# -*- mode: python -*-
# ET build script
# TTimo <ttimo@idsoftware.com>
# http://scons.sourceforge.net

import sys, os, string, commands, re #, time, re, pickle, StringIO, subprocess, pdb, zipfile, string, tempfile
import SCons



conf_filename = 'scons.conf'

# user options -----------------------------------

opts = Variables(conf_filename)
#opts.Add(EnumVariable('arch', 'Choose architecture to build for', 'linux-i386', allowed_values=('freebsd-i386', 'freebsd-amd64', 'linux-i386', 'linux-x86_64', 'netbsd-i386', 'opensolaris-i386', 'win32-mingw', 'darwin-ppc', 'darwin-i386')))
opts.Add(EnumVariable('warnings', 'Choose warnings level', '1', allowed_values=('0', '1', '2')))
opts.Add(EnumVariable('debug', 'Set to >= 1 to build for debug', '2', allowed_values=('0', '1', '2', '3')))
opts.Add(EnumVariable('optimize', 'Set to >= 1 to build with general optimizations', '2', allowed_values=('0', '1', '2', '3', '4', '5', '6')))
#opts.Add(EnumVariable('simd', 'Choose special CPU register optimizations', 'none', allowed_values=('none', 'sse', '3dnow')))
#opts.Add(EnumVariable('cpu', 'Set to 1 to build with special CPU register optimizations', 'i386', allowed_values=('i386', 'athlon-xp', 'core2duo')))
#opts.Add(BoolVariable('smp', 'Set to 1 to compile engine with symmetric multiprocessor support', 0))
#opts.Add(BoolVariable('java', 'Set to 1 to compile the engine and game with Java support', 0))
#opts.Add(BoolVariable('compatq3a', 'Set to 1 to compile engine with Q3A compatibility support', 0))
#opts.Add(BoolVariable('experimental', 'Set to 1 to compile engine with experimental features support', 0))
#opts.Add(BoolVariable('purevm', 'Set to 1 to compile the engine with strict checking for vm/*.qvm modules in paks', 0))
#opts.Add(BoolVariable('radiant', 'Set to 1 to compile the XreaLRadiant level editor', 0))
#opts.Add(BoolVariable('xmap', 'Set to 1 to compile the XMap(2) map compilers', 0))
#opts.Add(BoolVariable('vectorize', 'Set to 1 to compile the engine with auto-vectorization support', 0))
opts.Add(EnumVariable('curl', 'Choose http-download redirection support for the engine', 'system', allowed_values=('none', 'compile', 'system')))
#opts.Add(BoolVariable('openal', 'Set to 1 to compile the engine with OpenAL support', 0))
opts.Add(BoolVariable('noclient', 'Set to 1 to only compile the dedicated server', 0))
#opts.Add(BoolVariable('master', 'Set to 1 to compile the master server', 0))



# initialize compiler environment base ----------

env = Environment(ENV = {'PATH' : os.environ['PATH']}, options = opts, tools = ['default'])

Help(opts.GenerateHelpText(env))



# end help ---------------------------------------



# sanity -----------------------------------------

EnsureSConsVersion( 0, 96 )

# end sanity -------------------------------------



# system detection -------------------------------

# CPU type
cpu = commands.getoutput('uname -m')
dll_cpu = '???' # grmbl, alternative naming for .so

if cpu == 'x86_64' or cpu == 'amd64':
	cpu = 'x86_64'
	dll_cpu = 'x86_64'
elif re.compile('.*i?86.*').match(cpu):
	cpu = 'x86'
	dll_cpu = 'i386'
else:
	cpu = commands.getoutput('uname -p')
	if ( cpu == 'powerpc' ):
		cpu = 'ppc'
		dll_cpu = cpu
	else:
		cpu = 'cpu'
		dll_cpu = cpu
OS = commands.getoutput( 'uname -s' )
if ( OS == 'Darwin' ):
	print 'EXPERIMENTAL - INCOMPLETE'
	print 'Use the XCode projects to compile ET universal binaries for OSX'
	cpu = 'osx'
	dll_cpu = 'osx'

print 'cpu: ' + cpu

# end system detection ---------------------------




# arch detection ---------------------------------

def gcc_is_mingw():
	mingw = os.popen( env['CC'] + ' -dumpmachine' ).read()
	return re.search('mingw', mingw) != None

if gcc_is_mingw():
	g_os = 'win32'
elif OS == 'Darwin':
	g_os = 'Darwin'
else:
	g_os = 'Linux'
print 'OS: %s' % g_os

#if cpu == 'x86_64':
#	env['CC'] = 'gcc -m64'
#	env['CXX'] = 'g++ -m64'


# end arch detection -----------------------------



# command line settings --------------------------

for k in ARGUMENTS.keys():
	exec_cmd = k + '=\'' + ARGUMENTS[k] + '\''
	print 'Command line: ' + exec_cmd
	exec( exec_cmd )

# end command line settings ----------------------





# general configuration, target selection --------

if ( OS == 'Darwin' ):
	env.Append(CCFLAGS = '-D__MACOS__ -Wno-long-double -Wno-unknown-pragmas -Wno-trigraphs -fpascal-strings -fasm-blocks -Wreturn-type -Wunused-variable -ffast-math -fno-unsafe-math-optimizations -fvisibility=hidden -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk')
	env.Append(CCFLAGS = '-arch i386')
	env.Append(LDFLAGS = '-arch i386')

env.Append(CCFLAGS = '-pipe -fsigned-char')

if env['warnings'] == '1':
	env.Append(CCFLAGS = '-Wall -Wno-unused-parameter')
elif env['warnings'] == '2':
	env.Append(CCFLAGS = '-Wall -Werror')

if env['debug'] != '0':
	env.Append(CCFLAGS = '-ggdb${debug} -D_DEBUG -DDEBUG')
else:
	env.Append(CCFLAGS = '-DNDEBUG')

if env['optimize'] != '0':
	if ( OS == 'Linux' ):
		# -fomit-frame-pointer: gcc manual indicates -O sets this implicitely,
		# only if that doesn't affect debugging
		# on Linux, this affects backtrace capability, so I'm assuming this is needed
		# -finline-functions: implicit at -O3
		# -fschedule-insns2: implicit at -O3
		# -funroll-loops ?
		# -mfpmath=sse -msse ?
		if cpu == 'x86_64':
			env.Append(CCFLAGS = '-O${optimize} -Winline -ffast-math -fomit-frame-pointer -finline-functions -fschedule-insns2')
		else:
			env.Append(CCFLAGS = '-O${optimize} -march=i686 -Winline -ffast-math -fomit-frame-pointer -finline-functions -fschedule-insns2')


conf = Configure(env)
env = conf.Finish()


# save site configuration ----------------------

opts.Save(conf_filename, env)

# end save site configuration ------------------


if ( OS == 'Darwin' ):
	# configure for dynamic bundle
	env['SHLINKFLAGS'] = '$LINKFLAGS -bundle -flat_namespace -undefined suppress'
	env['SHLIBSUFFIX'] = '.so'

# maintain this dangerous optimization off at all times
env.Append( CPPFLAGS = '-fno-strict-aliasing' )



# mark the globals
# curl
local_dedicated = 0
local_curl = 0	# config selection
curl_lib = []

GLOBALS = 'env OS g_os cpu dll_cpu local_dedicated curl_lib'

# end general configuration ----------------------



# win32 cross compilation ----------------------

if g_os == 'win32' and os.name != 'nt':
	# mingw doesn't define the cpu type, but our only target is x86
	env.Append(CPPDEFINES = '_M_IX86=400')
	env.Append(LINKFLAGS = '-static-libgcc')
	# scons doesn't support cross-compiling, so set these up manually
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
	env['WIN32DEFSUFFIX']	= '.def'
	env['PROGSUFFIX']	= '.exe'
	env['SHLIBSUFFIX']	= '.dll'
	env['SHCCFLAGS']	= '$CCFLAGS'

# end win32 cross compilation ------------------



# targets ----------------------------------------

local_dedicated = 1
Export('GLOBALS ' + GLOBALS)
SConscript('SConscript_engine', variant_dir = 'build/dedicated', duplicate = 0)
	

# game logic -------------------------------------
SConscript('SConscript_etmain_game', variant_dir = 'build/etmain/game', duplicate = 0)

if env['noclient'] == 0:
	if env['curl'] == 'compile':
		SConscript('SConscript_curl')
	
	local_dedicated = 0
	Export('GLOBALS ' + GLOBALS)
	SConscript('SConscript_engine', variant_dir = 'build/engine', duplicate = 0)
	
	SConscript('SConscript_etmain_cgame', variant_dir = 'build/etmain/cgame', duplicate = 0)
	SConscript('SConscript_etmain_ui', variant_dir = 'build/etmain/ui', duplicate = 0)





#if ( TARGET_CORE == '1' ):
#	if ( DEDICATED == '0' or DEDICATED == '2' ):
#		local_dedicated = 0
#		Export( 'GLOBALS ' + GLOBALS )
		
#		VariantDir( g_build + '/core', '.', duplicate = 0 )
#		et = SConscript( g_build + '/core/SConscript_core' )
#		if ( g_os == 'win32' ):
#			toplevel_targets.append( InstallAs( '#ETXreaL.exe', et ) )
#		else:
#			toplevel_targets.append( InstallAs( '#etxreal', et ) )

#	if ( DEDICATED == '1' or DEDICATED == '2' ):
#		local_dedicated = 1
#		Export( 'GLOBALS ' + GLOBALS )
#		VariantDir( g_build + '/dedicated', '.', duplicate = 0 )
#		etded = SConscript( g_build + '/dedicated/SConscript_core' )
#		if ( g_os == 'win32' ):
#			toplevel_targets.append( InstallAs( '#ETXreaL-dedicated.exe', etded ) )
#		else:
#			toplevel_targets.append( InstallAs( '#etxreal-dedicated', etded ) )

#if ( TARGET_BSPC == '1' ):
#	Export( 'GLOBALS ' + GLOBALS )
#	VariantDir( g_build + '/bspc', '.', duplicate = 0 )
#	bspc = SConscript( g_build + '/bspc/SConscript_bspc' )
#	toplevel_targets.append( InstallAs( '#bspc.' + cpu, bspc ) )

#if ( TARGET_GAME == '1' ):
#	Export( 'GLOBALS ' + GLOBALS )
#	VariantDir( g_build + '/game', '.', duplicate = 0 )
#	game = SConscript( g_build + '/game/SConscript_game' )
#	toplevel_targets.append( InstallAs( '#etmain/qagame.mp.%s.so' % dll_cpu, game ) )

#if ( TARGET_CGAME == '1' ):
#	Export( 'GLOBALS ' + GLOBALS )
#	VariantDir( g_build + '/cgame', '.', duplicate = 0 )
#	cgame = SConscript( g_build + '/cgame/SConscript_cgame' )
#	toplevel_targets.append( InstallAs( '#etmain/cgame.mp.%s.so' % dll_cpu, cgame ) )

#if ( TARGET_UI == '1' ):
#	Export( 'GLOBALS ' + GLOBALS )
#	VariantDir( g_build + '/ui', '.', duplicate = 0 )
#	ui = SConscript( g_build + '/ui/SConscript.ui' )
#	toplevel_targets.append( InstallAs( '#etmain/ui.mp.%s.so' % dll_cpu, ui ) )

#class CopyBins(scons_utils.idSetupBase):
#	def copy_bins( self, target, source, env ):
#		for i in source:
#			j = os.path.normpath( os.path.join( os.path.dirname( i.abspath ), '../bin', os.path.basename( i.abspath ) ) )
#			self.SimpleCommand( 'cp ' + i.abspath + ' ' + j )
#			if ( OS == 'Linux' ):
#				self.SimpleCommand( 'strip ' + j )
#			else:
#				# see strip and otool man pages on mac
#				self.SimpleCommand( 'strip -ur ' + j )
#
#copybins_target = []
#if ( COPYBINS != '0' ):
#	copy = CopyBins()
#	copybins_target.append( Command( 'copybins', toplevel_targets, Action( copy.copy_bins ) ) )
#
#class MpBin(scons_utils.idSetupBase):
#	def mp_bin( self, target, source, env ):
#		temp_dir = tempfile.mkdtemp( prefix = 'mp_bin' )
#		self.SimpleCommand( 'cp ../bin/ui* ' + temp_dir )
#		self.SimpleCommand( 'cp ../bin/cgame* ' + temp_dir )
#		# zip the mac bundles
#		mac_bundle_dir = tempfile.mkdtemp( prefix = 'mp_mac' )
#		self.SimpleCommand( 'cp -R "../bin/Wolfenstein ET.app/Contents/Resources/ui_mac.bundle" ' + mac_bundle_dir )
#		self.SimpleCommand( 'cp -R "../bin/Wolfenstein ET.app/Contents/Resources/cgame_mac.bundle" ' + mac_bundle_dir )
#		self.SimpleCommand( 'find %s -name \.svn | xargs rm -rf' % mac_bundle_dir )
#		self.SimpleCommand( 'cd %s ; zip -r -D %s/ui_mac.zip ui_mac.bundle ; mv %s/ui_mac.zip %s/ui_mac' % ( mac_bundle_dir, temp_dir, temp_dir, temp_dir ) )
#		self.SimpleCommand( 'cd %s ; zip -r -D %s/cgame_mac.zip cgame_mac.bundle ; mv %s/cgame_mac.zip %s/cgame_mac' % ( mac_bundle_dir, temp_dir, temp_dir, temp_dir ) )
#		mp_bin_path = os.path.abspath( os.path.join ( os.getcwd(), '../etmain/mp_bin.pk3' ) )
#		self.SimpleCommand( 'cd %s ; zip -r -D %s *' % ( temp_dir, mp_bin_path ) )
#
#if ( BUILDMPBIN != '0' ):
#	mp_bin = MpBin()
#	mpbin_target = Command( 'mp_bin', toplevel_targets + copybins_target, Action( mp_bin.mp_bin ) )

# end targets ------------------------------------
