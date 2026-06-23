#!/Applications/klayout.app/Contents/MacOS/klayout -b -r
import code
import os
import sys

pwd = os.getcwd()
sys.path.append(pwd)

rootPython = "/Applications/klayout.app/Contents/Frameworks/Python.framework/Versions"
verPython  = "${PYTHON_VER}"
piptarget  = [ "--target", f"{rootPython}/{verPython}/lib/python{verPython}/site-packages" ]

piphelpstr = """
--------------------------------------------------------------------------------
(1) Install ['pandas', 'scipy', 'matplotlib']
>>> import pip
>>> pip.main( ['install', 'pandas', 'scipy', 'matplotlib'] + piptarget )

(2) List modules
>>> import pip
>>> pip.main( ['list'] )

(3) Uninstall ['scipy']
>>> import pip
>>> pip.main( ['uninstall', 'scipy'] )
--------------------------------------------------------------------------------
"""
def howtopip():
    print(piphelpstr)


variables = globals().copy()
variables.update(locals())
shell = code.InteractiveConsole(variables)
cprt = 'Type "help", "copyright", "credits" or "license" for more information.'
banner = "Python {} on {}\n{}\n({})".format(sys.version, sys.platform,
    cprt, "KLayout Python Console")
exit_msg = 'now exiting {}...'.format("KLayout Python Console")
shell.interact(banner, exit_msg)
