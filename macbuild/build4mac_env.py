#!/usr/bin/env python3

#===============================================================================
# File: "macbuild/build4mac_env.py"
#
# Here are dictionaries of ...
#  different modules for building KLayout (http://www.klayout.de/index.php)
#  version 0.30.5 or later on different Apple Mac OSX platforms.
#
# This file is imported by the 'build4mac.py' script.
#===============================================================================
import glob
import os
import platform
import re

#---------------------------------------------------------------------------------------------------
# [0] Xcode's tools
#       and
#     Default Homebrew root
#       Ref. https://github.com/Homebrew/brew/blob/master/docs/Installation.md#alternative-installs
#---------------------------------------------------------------------------------------------------
XcodeToolChain = { 'nameID': '/usr/bin/install_name_tool -id ',
                   'nameCH': '/usr/bin/install_name_tool -change ',
                 }

(System, Node, Release, MacVersion, Machine, Processor) = platform.uname()
if Machine == "arm64": # Apple Silicon!
  DefaultHomebrewRoot  = '/opt/homebrew'
  DefaultAnaconda3Root = '/opt/anaconda3'
  Ana3VirEnv5          = f'{DefaultAnaconda3Root}/envs/klayout-qt5'
  Ana3VirEnv6          = f'{DefaultAnaconda3Root}/envs/klayout-qt6'
  HomebrewSearchPathFilter1 = f'\t+{DefaultHomebrewRoot}/opt'
  HomebrewSearchPathFilter2 = '\t+@loader_path/../../../../../../../../../../opt'
  HomebrewSearchPathFilter3 =    '@loader_path/../../../../../../../../../../opt' # no leading white space
  # 1: absolute path as seen in ~python@3.9.17
  # 2: relative path as seen in  python@3.9.18
else: # x86_64|Intel
  DefaultHomebrewRoot  = '/usr/local'
  DefaultAnaconda3Root = '/Applications/anaconda3'
  Ana3VirEnv5          = f'{DefaultAnaconda3Root}/envs/klayout-qt5'
  Ana3VirEnv6          = f'{DefaultAnaconda3Root}/envs/klayout-qt6'
  HomebrewSearchPathFilter1 = f'\t+{DefaultHomebrewRoot}/opt'
  HomebrewSearchPathFilter2 = '\t+@loader_path/../../../../../../../../../../opt'
  HomebrewSearchPathFilter3 =    '@loader_path/../../../../../../../../../../opt' # no leading white space
  # 1: absolute path as seen in ~python@3.9.17
  #      BigSur{kazzz-s} lib-dynload (1)% otool -L _sqlite3.cpython-39-darwin.so
  #      _sqlite3.cpython-39-darwin.so:
  #   ===> /usr/local/opt/sqlite/lib/libsqlite3.0.dylib (compatibility version 9.0.0, current version 9.6.0)
  #        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1292.100.5)
  #
  # 2: relative path as seen in python@3.9.18
  #      Monterey{kazzz-s} lib-dynload (1)% otool -L _sqlite3.cpython-39-darwin.so
  #      _sqlite3.cpython-39-darwin.so:
  #   ===> @loader_path/../../../../../../../../../../opt/sqlite/lib/libsqlite3.0.dylib (compatibility version 9.0.0, current version 9.6.0)
  #        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1311.100.3)
  #
  # 3. absolute path again as seen in python@3.11
  #      Monterey{kazzz-s} lib-dynload (1)% otool -L _sqlite3.cpython-311-darwin.so
  #      _sqlite3.cpython-311-darwin.so:
  #   ===> /usr/local/opt/sqlite/lib/libsqlite3.0.dylib (compatibility version 9.0.0, current version 9.6.0)
  #        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1311.100.3)
  #
  # Ref. https://github.com/Homebrew/homebrew-core/issues/140930#issuecomment-1701524467
del System, Node, Release, MacVersion, Machine, Processor

#-----------------------------------------------------
# [1] Qt5 or Qt6
#-----------------------------------------------------
Qts  = [ 'Qt5MacPorts', 'Qt5Brew', 'Qt5Ana3' ]
Qts += [ 'Qt6MacPorts', 'Qt6Brew', 'Qt6Ana3' ]

#-----------------------------------------------------
# Whereabouts of different components of Qt5
#-----------------------------------------------------
# Qt5 from MacPorts (https://www.macports.org/)
#   install with 'sudo port install [qt5|qt5-qttools]'
# [Key Type Name] = 'Qt5MacPorts'
Qt5MacPorts = { 'qmake' : '/opt/local/libexec/qt5/bin/qmake',
                'deploy': '/opt/local/libexec/qt5/bin/macdeployqt',
                'libdir': '/opt/local/libexec/qt5/lib',
              }

# Qt5 from Homebrew (https://brew.sh/)
#   install with 'brew install qt5'
# [Key Type Name] = 'Qt5Brew'
Qt5Brew = { 'qmake' : f'{DefaultHomebrewRoot}/opt/qt@5/bin/qmake',
            'deploy': f'{DefaultHomebrewRoot}/opt/qt@5/bin/macdeployqt',
            'libdir': f'{DefaultHomebrewRoot}/opt/qt@5/lib',
          }

#---------------------------------------------------------------------------------------------------
# [Apple Silicon]
#   Qt5 is to be installed under /opt/anaconda3/envs/klayout-qt5
#   after installing "Anaconda3-2025.06-0-MacOSX-arm64.pkg" under /opt/anaconda3/.
#
#   1) Create a new env "klayout-qt5" (with stable solver & channels)
#      switch solver to libmamba for faster/more stable resolves
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#      Create the environment (on this ARM machine it will pull osx-arm64 builds)
#        $ conda create -n klayout-qt5 python=3.13 -y
#        $ conda activate klayout-qt5
#
#      In this env only, prefer conda-forge strictly (to avoid mixing)
#        $ conda config --env --add channels conda-forge
#        $ conda config --env --add channels defaults
#        $ conda config --env --set channel_priority strict
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#   2) Install Qt5 (qt-main) only from conda-forge
#      Qt5 core (builds that typically include Designer/UiTools)
#        $ conda install -y --override-channels -c conda-forge "qt-main=5.15.15"
#
#   3) Additionally, install Ruby and libgit2 only from conda-forge
#        $ conda install -y --override-channels -c conda-forge "ruby=3.4.7"
#        $ conda install -y --override-channels -c conda-forge "libgit2=1.9.1"
#---------------------------------------------------------------------------------------------------
# [x86_64|Intel]
#   Qt5 is to be installed under /Applications/anaconda3/envs/klayout-qt5
#   after installing "Anaconda3-2025.06-0-MacOSX-x86_64.pkg" under /Applications/anaconda3/.
#
#   1) Create a new env "klayout-qt5" (with stable solver & channels)
#      switch solver to libmamba for faster/more stable resolves
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#      Create the environment (on this x86_64 machine it will pull osx-64 builds)
#        $ conda create -n klayout-qt5 python=3.13 -y
#        $ conda activate klayout-qt5
#
#      In this env only, prefer conda-forge strictly (to avoid mixing)
#        $ conda config --env --add channels conda-forge
#        $ conda config --env --add channels defaults
#        $ conda config --env --set channel_priority strict
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#   2) Install Qt5 (qt-main) only from conda-forge
#      Qt5 core (builds that typically include Designer/UiTools)
#        $ conda install -y --override-channels -c conda-forge "qt-main=5.15.15"
#
#   3) Additionally, install Ruby and libgit2 only from conda-forge
#        $ conda install -y --override-channels -c conda-forge "ruby=3.4.7"
#        $ conda install -y --override-channels -c conda-forge "libgit2=1.9.1"
#---------------------------------------------------------------------------------------------------
# [Key Type Name] = 'Qt5Ana3'
Qt5Ana3 = { 'qmake' : f'{Ana3VirEnv5}/bin/qmake',
            'deploy': f'{Ana3VirEnv5}/bin/macdeployqt',
            'libdir': f'{Ana3VirEnv5}/lib',
          }

#-------------------------------------------------------------------------
# Whereabouts of different components of Qt6    *+*+*+ EXPERIMENTAL *+*+*+
#-------------------------------------------------------------------------
# Qt6 from MacPorts (https://www.macports.org/)
#   install with 'sudo port install [qt6|qt6-qttools]'
# [Key Type Name] = 'Qt6MacPorts'
Qt6MacPorts = { 'qmake' : '/opt/local/libexec/qt6/bin/qmake',
                'deploy': '/opt/local/libexec/qt6/bin/macdeployqt',
                'libdir': '/opt/local/libexec/qt6/lib',
              }

# Qt6 from Homebrew (https://brew.sh/)
#   install with 'brew install qt6'
# [Key Type Name] = 'Qt6Brew'
Qt6Brew = { 'qmake' : f'{DefaultHomebrewRoot}/opt/qt@6/bin/qmake',
            'deploy': f'{DefaultHomebrewRoot}/opt/qt@6/bin/macdeployqt',
            'libdir': f'{DefaultHomebrewRoot}/opt/qt@6/lib',
          }

#---------------------------------------------------------------------------------------------------
# [Apple Silicon]
#   Qt6 is to be installed under /opt/anaconda3/envs/klayout-qt6
#   after installing "Anaconda3-2025.06-0-MacOSX-arm64.pkg" under /opt/anaconda3/.
#
#   1) Create a new env "klayout-qt6" (with stable solver & channels)
#      switch solver to libmamba for faster/more stable resolves
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#      Create the environment (on this ARM machine it will pull osx-arm64 builds)
#        $ conda create -n klayout-qt6 python=3.13 -y
#        $ conda activate klayout-qt6
#
#      In this env only, prefer conda-forge strictly (to avoid mixing)
#        $ conda config --env --add channels conda-forge
#        $ conda config --env --add channels defaults
#        $ conda config --env --set channel_priority strict
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#   2) Install Qt6 (qt6-main and qt6-multimedia) only from conda-forge
#      Qt6 core (builds that typically include Designer/UiTools)
#        $ conda install -y --override-channels -c conda-forge "qt6-main=6.9.3"
#        $ conda install -y --override-channels -c conda-forge "qt6-multimedia=6.9.3"
#
#   3) Additionally, install Ruby and libgit2 only from conda-forge
#        $ conda install -y --override-channels -c conda-forge "ruby=3.4.7"
#        $ conda install -y --override-channels -c conda-forge "libgit2=1.9.1"
#---------------------------------------------------------------------------------------------------
# [x86_64|Intel]
#   Qt6 is to be installed under /Applications/anaconda3/envs/klayout-qt6
#   after installing "Anaconda3-2025.06-0-MacOSX-x86_64.pkg" under /Applications/anaconda3/.
#
#   1) Create a new env "klayout-qt6" (with stable solver & channels)
#      switch solver to libmamba for faster/more stable resolves
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#      Create the environment (on this x86_64 machine it will pull osx-64 builds)
#        $ conda create -n klayout-qt6 python=3.13 -y
#        $ conda activate klayout-qt6
#
#      In this env only, prefer conda-forge strictly (to avoid mixing)
#        $ conda config --env --add channels conda-forge
#        $ conda config --env --add channels defaults
#        $ conda config --env --set channel_priority strict
#        $ conda install -n base -y conda-libmamba-solver
#        $ conda config --set solver libmamba
#
#   2) Install Qt6 (qt6-main and qt6-multimedia) only from conda-forge
#      Qt6 core (builds that typically include Designer/UiTools)
#        $ conda install -y --override-channels -c conda-forge "qt6-main=6.9.3"
#        $ conda install -y --override-channels -c conda-forge "qt6-multimedia=6.9.3"
#
#   3) Additionally, install Ruby and libgit2 only from conda-forge
#        $ conda install -y --override-channels -c conda-forge "ruby=3.4.7"
#        $ conda install -y --override-channels -c conda-forge "libgit2=1.9.1"
#---------------------------------------------------------------------------------------------------
# [Key Type Name] = 'Qt6Ana3'
Qt6Ana3 = { 'qmake' : f'{Ana3VirEnv6}/bin/qmake6',
            'deploy': f'{Ana3VirEnv6}/bin/macdeployqt6',
            'libdir': f'{Ana3VirEnv6}/lib',
          }

# Consolidated dictionary kit for Qt[5|6]
Qt56Dictionary  = { 'Qt5MacPorts': Qt5MacPorts,
                    'Qt5Brew'    : Qt5Brew,
                    'Qt5Ana3'    : Qt5Ana3,
                    'Qt6MacPorts': Qt6MacPorts,
                    'Qt6Brew'    : Qt6Brew,
                    'Qt6Ana3'    : Qt6Ana3,
                  }

#-----------------------------------------------------
# [2] Ruby
#     * Dropped the followings (2023-10-24).
#         Sys: [ElCapitan - BigSur]
#         Ext: [Ruby31]
#     * See 415b5aa2efca04928f1148a69e77efd5d76f8c1d
#           for the previous states.
#-----------------------------------------------------
RubyNil  = [ 'nil' ]
RubySys  = [ 'RubyMonterey', 'RubyVentura', 'RubySonoma', 'RubySequoia', 'RubyTahoe' ]
RubyExt  = [ 'Ruby34MacPorts', 'Ruby34Brew', 'RubyAnaconda3' ]
Rubies   = RubyNil + RubySys + RubyExt

#-----------------------------------------------------
# Whereabouts of different components of Ruby
#-----------------------------------------------------
# % which ruby
#   /System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/bin/ruby
#
# % ruby -v
#   ruby 2.6.10p210 (2022-04-12 revision 67958) [universal.x86_64-darwin21]
#
# Where is the 'ruby.h' used to build the 'ruby' executable?
#
# % ruby -e "puts File.expand_path('ruby.h', RbConfig::CONFIG['rubyhdrdir'])"
#   ===> /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX13.1.sdk \
#        /System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/include/ruby-2.6.0/ruby.h
#
# Bundled with Monterey (12.x)
# [Key Type Name] = 'Sys'
MontereyXcSDK   = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
MontereyCLTSDK  = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
RubyMonterey    = { 'exe':  '/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/bin/ruby',
                    'inc':  f'{MontereyXcSDK}/System/Library/Frameworks/Ruby.framework/Headers',
                    'inc2': f'{MontereyXcSDK}/System/Library/Frameworks/Ruby.framework/Headers/ruby',
                    'lib':  f'{MontereyXcSDK}/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/lib/libruby.tbd',
                  }

# Bundled with Ventura (13.x)
# [Key Type Name] = 'Sys'
VenturaXcSDK    = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
VenturaCLTSDK   = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
RubyVentura     = { 'exe':  '/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/bin/ruby',
                    'inc':  f'{VenturaXcSDK}/System/Library/Frameworks/Ruby.framework/Headers',
                    'inc2': f'{VenturaXcSDK}/System/Library/Frameworks/Ruby.framework/Headers/ruby',
                    'lib':  f'{VenturaXcSDK}/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/lib/libruby.tbd',
                  }

# Bundled with Sonoma (14.x)
# [Key Type Name] = 'Sys'
SonomaXcSDK     = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
SonomaCLTSDK    = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
RubySonoma      = { 'exe':  '/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/bin/ruby',
                    'inc':  f'{SonomaXcSDK}/System/Library/Frameworks/Ruby.framework/Headers',
                    'inc2': f'{SonomaXcSDK}/System/Library/Frameworks/Ruby.framework/Headers/ruby',
                    'lib':  f'{SonomaXcSDK}/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/lib/libruby.tbd',
                  }

# Bundled with Sequoia (15.x)
# [Key Type Name] = 'Sys'
SequoiaXcSDK     = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
SequoiaCLTSDK    = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
RubySequoia      = { 'exe':  '/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/bin/ruby',
                     'inc':  f'{SequoiaXcSDK}/System/Library/Frameworks/Ruby.framework/Headers',
                     'inc2': f'{SequoiaXcSDK}/System/Library/Frameworks/Ruby.framework/Headers/ruby',
                     'lib':  f'{SequoiaXcSDK}/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/lib/libruby.tbd',
                   }

# Bundled with Tahoe (26.x)
# [Key Type Name] = 'Sys'
TahoeXcSDK       = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
TahoeCLTSDK      = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
RubyTahoe        = { 'exe':  '/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/bin/ruby',
                     'inc':  f'{TahoeXcSDK}/System/Library/Frameworks/Ruby.framework/Headers',
                     'inc2': f'{TahoeXcSDK}/System/Library/Frameworks/Ruby.framework/Headers/ruby',
                     'lib':  f'{TahoeXcSDK}/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/lib/libruby.tbd',
                   }

# Ruby 3.4 from MacPorts (https://www.macports.org/)
#  install with 'sudo port install ruby34'
# [Key Type Name] = 'MP34'
Ruby34MacPorts  = { 'exe': '/opt/local/bin/ruby3.4',
                    'inc': '/opt/local/include/ruby-3.4.9',
                    'lib': '/opt/local/lib/libruby.3.4.dylib',
                  }

# Ruby 3.4 from Homebrew
#   install with 'brew install ruby@3.4'
# [Key Type Name] = 'HB34'
HBRuby34Path    = f'{DefaultHomebrewRoot}/opt/ruby@3.4'
Ruby34Brew      = { 'exe': f'{HBRuby34Path}/bin/ruby',
                    'inc': f'{HBRuby34Path}/include/ruby-3.4.0',
                    'lib': f'{HBRuby34Path}/lib/libruby.3.4.dylib',
                  }

# Ruby 3.4 installed under [/opt|/Applications]/anaconda3/envs/klayout-qt[5|6]
#   See the [Qt5] [Qt6] section above.
# [Key Type Name] = 'Ana3'
RubyAnaconda3V5   = { 'exe': f'{Ana3VirEnv5}/bin/ruby',
                      'inc': f'{Ana3VirEnv5}/include/ruby-3.4.0',
                      'lib': f'{Ana3VirEnv5}/lib/libruby.3.4.dylib',
                    }

RubyAnaconda3V6   = { 'exe': f'{Ana3VirEnv6}/bin/ruby',
                      'inc': f'{Ana3VirEnv6}/include/ruby-3.4.0',
                      'lib': f'{Ana3VirEnv6}/lib/libruby.3.4.dylib',
                    }

# Consolidated dictionary kit for Ruby
RubyDictionary  = { 'nil'             : None,
                    'RubyMonterey'    : RubyMonterey,
                    'RubyVentura'     : RubyVentura,
                    'RubySonoma'      : RubySonoma,
                    'RubySequoia'     : RubySequoia,
                    'RubyTahoe'       : RubyTahoe,
                    'Ruby34MacPorts'  : Ruby34MacPorts,
                    'Ruby34Brew'      : Ruby34Brew,
                    'RubyAnaconda3V5' : RubyAnaconda3V5,
                    'RubyAnaconda3V6' : RubyAnaconda3V6,
                  }

#-----------------------------------------------------
# [3] Python
#     * Dropped the followings (2023-10-24).
#         Sys: [ElCapitan - Monterey]
#         Ext: [Python38]
#     * See 415b5aa2efca04928f1148a69e77efd5d76f8c1d
#           for the previous states.
#-----------------------------------------------------
PythonNil  = [ 'nil' ]
PythonSys  = [ 'PythonMonterey', 'PythonVentura', 'PythonSonoma', 'PythonSequoia', 'PythonTahoe' ]
PythonExt  = [ 'Python311MacPorts', 'Python312MacPorts', 'Python313MacPorts' ]
PythonExt += [ 'Python311Brew', 'Python312Brew', 'Python313Brew', 'PythonAutoBrew' ]
PythonExt += [ 'PythonAnaconda3' ]
Pythons    = PythonNil + PythonSys + PythonExt

#-----------------------------------------------------
# Whereabouts of different components of Python
#-----------------------------------------------------
# Bundled with Monterey (12.x)
# [Key Type Name] = 'Sys'
MontereyPy3FWXc = "/Applications/Xcode.app/Contents/Developer/Library/Frameworks"
MontereyPy3FW   = "/Library/Developer/CommandLineTools/Library/Frameworks"
PythonMonterey  = { 'exe': f'{MontereyPy3FW}/Python3.framework/Versions/3.9/bin/python3.9',
                    'inc': f'{MontereyPy3FW}/Python3.framework/Versions/3.9/include/python3.9',
                    'lib': f'{MontereyPy3FW}/Python3.framework/Versions/3.9/lib/libpython3.9.dylib',
                  }

# Bundled with Ventura (13.x)
# [Key Type Name] = 'Sys'
VenturaPy3FWXc  = "/Applications/Xcode.app/Contents/Developer/Library/Frameworks"
VenturaPy3FW    = "/Library/Developer/CommandLineTools/Library/Frameworks"
PythonVentura   = { 'exe': f'{VenturaPy3FW}/Python3.framework/Versions/3.9/bin/python3.9',
                    'inc': f'{VenturaPy3FW}/Python3.framework/Versions/3.9/include/python3.9',
                    'lib': f'{VenturaPy3FW}/Python3.framework/Versions/3.9/lib/libpython3.9.dylib',
                  }

# Bundled with Sonoma (14.x)
# [Key Type Name] = 'Sys'
SonomaPy3FWXc   = "/Applications/Xcode.app/Contents/Developer/Library/Frameworks"
SonomaPy3FW     = "/Library/Developer/CommandLineTools/Library/Frameworks"
PythonSonoma    = { 'exe': f'{SonomaPy3FW}/Python3.framework/Versions/3.9/bin/python3.9',
                    'inc': f'{SonomaPy3FW}/Python3.framework/Versions/3.9/include/python3.9',
                    'lib': f'{SonomaPy3FW}/Python3.framework/Versions/3.9/lib/libpython3.9.dylib',
                  }

# Bundled with Sequoia (15.x)
# [Key Type Name] = 'Sys'
SequoiaPy3FWXc   = "/Applications/Xcode.app/Contents/Developer/Library/Frameworks"
SequoiaPy3FW     = "/Library/Developer/CommandLineTools/Library/Frameworks"
PythonSequoia    = { 'exe': f'{SequoiaPy3FW}/Python3.framework/Versions/3.9/bin/python3.9',
                     'inc': f'{SequoiaPy3FW}/Python3.framework/Versions/3.9/include/python3.9',
                     'lib': f'{SequoiaPy3FW}/Python3.framework/Versions/3.9/lib/libpython3.9.dylib',
                   }

# Bundled with Tahoe (26.x)
# [Key Type Name] = 'Sys'
TahoePy3FWXc     = "/Applications/Xcode.app/Contents/Developer/Library/Frameworks"
TahoePy3FW       = "/Library/Developer/CommandLineTools/Library/Frameworks"
PythonTahoe      = { 'exe': f'{TahoePy3FW}/Python3.framework/Versions/3.9/bin/python3.9',
                     'inc': f'{TahoePy3FW}/Python3.framework/Versions/3.9/include/python3.9',
                     'lib': f'{TahoePy3FW}/Python3.framework/Versions/3.9/lib/libpython3.9.dylib',
                   }

# Python 3.11 from MacPorts (https://www.macports.org/)
#   install with 'sudo port install python311'
# [Key Type Name] = 'MP311'
Python311MacPorts = { 'exe': '/opt/local/Library/Frameworks/Python.framework/Versions/3.11/bin/python3.11',
                      'inc': '/opt/local/Library/Frameworks/Python.framework/Versions/3.11/include/python3.11',
                      'lib': '/opt/local/Library/Frameworks/Python.framework/Versions/3.11/lib/libpython3.11.dylib',
                    }

# Python 3.12 from MacPorts (https://www.macports.org/)
#   install with 'sudo port install python312'
# [Key Type Name] = 'MP312'
Python312MacPorts = { 'exe': '/opt/local/Library/Frameworks/Python.framework/Versions/3.12/bin/python3.12',
                      'inc': '/opt/local/Library/Frameworks/Python.framework/Versions/3.12/include/python3.12',
                      'lib': '/opt/local/Library/Frameworks/Python.framework/Versions/3.12/lib/libpython3.12.dylib',
                    }

# Python 3.13 from MacPorts (https://www.macports.org/)
#   install with 'sudo port install python313'
# [Key Type Name] = 'MP313'
Python313MacPorts = { 'exe': '/opt/local/Library/Frameworks/Python.framework/Versions/3.13/bin/python3.13',
                      'inc': '/opt/local/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13',
                      'lib': '/opt/local/Library/Frameworks/Python.framework/Versions/3.13/lib/libpython3.13.dylib',
                    }

# Python 3.11 from Homebrew
#   install with 'brew install python@3.11'
# [Key Type Name] = 'HB311'
HBPython311FrameworkPath = f'{DefaultHomebrewRoot}/opt/python@3.11/Frameworks/Python.framework'
Python311Brew     = { 'exe': f'{HBPython311FrameworkPath}/Versions/3.11/bin/python3.11',
                      'inc': f'{HBPython311FrameworkPath}/Versions/3.11/include/python3.11',
                      'lib': f'{HBPython311FrameworkPath}/Versions/3.11/lib/libpython3.11.dylib',
                    }

# Python 3.12 from Homebrew
#   install with 'brew install python@3.12'
# [Key Type Name] = 'HB312'
HBPython312FrameworkPath = f'{DefaultHomebrewRoot}/opt/python@3.12/Frameworks/Python.framework'
Python312Brew     = { 'exe': f'{HBPython312FrameworkPath}/Versions/3.12/bin/python3.12',
                      'inc': f'{HBPython312FrameworkPath}/Versions/3.12/include/python3.12',
                      'lib': f'{HBPython312FrameworkPath}/Versions/3.12/lib/libpython3.12.dylib',
                    }

# Python 3.13 from Homebrew
#   install with 'brew install python@3.13'
# [Key Type Name] = 'HB313'
HBPython313FrameworkPath = f'{DefaultHomebrewRoot}/opt/python@3.13/Frameworks/Python.framework'
Python313Brew     = { 'exe': f'{HBPython313FrameworkPath}/Versions/3.13/bin/python3.13',
                      'inc': f'{HBPython313FrameworkPath}/Versions/3.13/include/python3.13',
                      'lib': f'{HBPython313FrameworkPath}/Versions/3.13/lib/libpython3.13.dylib',
                    }

# Python 3.13 installed under [/opt|/Applications]/anaconda3/klayout-qt[5|6]
#   See the [Qt5] [Qt6] section above.
# [Key Type Name] = 'Ana3'
PythonAnaconda3V5 = { 'exe': f'{Ana3VirEnv5}/bin/python3.13',
                      'inc': f'{Ana3VirEnv5}/include/python3.13',
                      'lib': f'{Ana3VirEnv5}/lib/libpython3.13.dylib',
                    }

PythonAnaconda3V6 = { 'exe': f'{Ana3VirEnv6}/bin/python3.13',
                      'inc': f'{Ana3VirEnv6}/include/python3.13',
                      'lib': f'{Ana3VirEnv6}/lib/libpython3.13.dylib',
                    }

# Latest Python from Homebrew
#   install with 'brew install python'
#   There can be multiple candidates such as: (python, python3, python@3, python@3.8, python@3.9,
#                                              python@3.10, python@3.12, python@3.12, python@3.13 )
#   Hard to tell which is going to be available to the user. Picking the last one.
# [Key Type Name] = 'HBAuto'
HBPythonAutoFrameworkPath = ""
HBPythonAutoVersion       = ""
try:
    patPF = rf"({DefaultHomebrewRoot}/opt/python)([@]?)(3[.]?)([0-9]*)(/Frameworks/Python[.]framework)"
    regPF = re.compile(patPF)
    dicPy = dict()
    for item in glob.glob( f"{DefaultHomebrewRoot}/opt/python*/Frameworks/Python.framework" ):
        if regPF.match(item):
            pyver = regPF.match(item).groups()[3] # ([0-9]*)
            if pyver == "":
                pyver = "0"
            dicPy[ int(pyver) ] = ( item, "3."+pyver )
    keys = sorted( dicPy.keys(), reverse=True )
    HBPythonAutoFrameworkPath = dicPy[keys[0]][0]
    HBPythonAutoVersion       = dicPy[keys[0]][1]

    HBAutoFrameworkVersionPath, HBPythonAutoVersion = os.path.split( glob.glob( f"{HBPythonAutoFrameworkPath}/Versions/3*" )[0] )
    PythonAutoBrew  = { 'exe': f'{HBAutoFrameworkVersionPath}/{HBPythonAutoVersion}/bin/python{HBPythonAutoVersion}',
                        'inc': f'{HBAutoFrameworkVersionPath}/{HBPythonAutoVersion}/include/python{HBPythonAutoVersion}',
                        'lib': glob.glob( f"{HBAutoFrameworkVersionPath}/{HBPythonAutoVersion}/lib/*.dylib" )[0],
                      }
    """
    # when I have [python3, python@3, python@3.11, python@3.12]
    print(HBPythonAutoFrameworkPath)
    print(HBAutoFrameworkVersionPath)
    print(HBPythonAutoVersion)
    print(PythonAutoBrew)
    quit()

    /usr/local/opt/python@3.12/Frameworks/Python.framework
    /usr/local/opt/python@3.12/Frameworks/Python.framework/Versions
    3.12
    { 'exe': '/usr/local/opt/python@3.12/Frameworks/Python.framework/Versions/3.12/bin/python3.12',
      'inc': '/usr/local/opt/python@3.12/Frameworks/Python.framework/Versions/3.12/include/python3.12',
      'lib': '/usr/local/opt/python@3.12/Frameworks/Python.framework/Versions/3.12/lib/libpython3.12.dylib'
    }
    """
except Exception:
    _have_Homebrew_Python = False
    print( "  WARNING!!! Since you don't have the Homebrew Python Frameworks, you cannot use the '-p HBAuto' option. " )
    pass
else:
    _have_Homebrew_Python = True

# Consolidated dictionary kit for Python
PythonDictionary = { 'nil'                : None,
                     'PythonMonterey'     : PythonMonterey,
                     'PythonVentura'      : PythonVentura,
                     'PythonSonoma'       : PythonSonoma,
                     'PythonSequoia'      : PythonSequoia,
                     'PythonTahoe'        : PythonTahoe,
                     'Python313MacPorts'  : Python313MacPorts,
                     'Python313Brew'      : Python313Brew,
                     'PythonAnaconda3V5'  : PythonAnaconda3V5,
                     'PythonAnaconda3V6'  : PythonAnaconda3V6,
                     'Python312MacPorts'  : Python312MacPorts,
                     'Python312Brew'      : Python312Brew,
                     'Python311MacPorts'  : Python311MacPorts,
                     'Python311Brew'      : Python311Brew,
                   }
if _have_Homebrew_Python:
    PythonDictionary['PythonAutoBrew'] = PythonAutoBrew

#-----------------------------------------------------
# [4] KLayout executables including buddy tools
#-----------------------------------------------------
KLayoutExecs  = [ 'klayout' ]
KLayoutExecs += [ 'strm2cif', 'strm2dxf', 'strm2gds', 'strm2gdstxt', 'strm2lstr', 'strm2mag' ]
KLayoutExecs += [ 'strm2oas', 'strm2txt', 'strmclip', 'strmcmp',     'strmrun',   'strmxor'  ]

#----------------
# End of File
#----------------
