#!/Applications/anaconda3/bin/python3

import glob
import optparse
import os
import platform
import shutil
import subprocess
import sys

#------------------------------------------------------------------------------
# In general, avoid setting the first line to '#!/usr/bin/env python3'.
# If so, when this script is invoked in the 'KLayoutNightlyBuild.app' script
# bundle created by Automator, the python3 will be the macOS-bundled python3,
# where pandas is not included by default.
# Therefore, it is better to use one of:
#   1) #!/Applications/anaconda3/bin/python3 (Anaconda3)
#   2) #!/usr/local/bin/python3 (Homebrew needs 'pip3 install pandas')
#   3) #!/opt/local/bin/python3 (MacPorts needs 'sudo pip3 install pandas')
#
# However, if we install 'pandas' and its dependencies to the system Python
# environment, we can also set '#!/usr/bin/env python3'.
#------------------------------------------------------------------------------
import pandas as pd


#------------------------------------------------------------------------------
## To test if the platform is a member of valid platforms
#
# @param[in] platforms     valid platforms
#
# @return matching platform name on success; "" on failure
#------------------------------------------------------------------------------
def Test_My_Platform( platforms=None ):
    if platforms is None:
        platforms = ['Monterey', 'Ventura', 'Sonoma', 'Sequoia', 'Tahoe']
    (System, Node, Release, MacVersion, Machine, Processor) = platform.uname()

    if System != "Darwin":
        return ""

    release = int( Release.split(".")[0] ) # take the first of ['21', '0', '0']
    if   release == 25:
        Platform = "Tahoe"
    elif release == 24:
        Platform = "Sequoia"
    elif release == 23:
        Platform = "Sonoma"
    elif release == 22:
        Platform = "Ventura"
    elif release == 21:
        Platform = "Monterey"
    else:
        Platform = ""

    if Platform in platforms:
        return Platform
    return ""

#------------------------------------------------------------------------------
## To populate the build target dictionary
#
# @return a dictionary; key=integer, value=mnemonic
#------------------------------------------------------------------------------
def Get_Build_Target_Dict():
    buildTargetDic     = dict()
    buildTargetDic[0]  = 'std'
    buildTargetDic[1]  = 'ports'
    buildTargetDic[2]  = 'brew'
    buildTargetDic[3]  = 'brewHW'
    buildTargetDic[4]  = 'ana3'

    buildTargetDic[5]  = 'brewA'
    buildTargetDic[6]  = 'brewAHW'

    buildTargetDic[12] = 'pbrew'   # use MacPorts' Qt and Homebrew's (Ruby, Python)
    buildTargetDic[13] = 'pbrewHW' # use MacPorts' Qt and Homebrew's Python
    return buildTargetDic

#------------------------------------------------------------------------------
## To get the build option dictionary
#
# @param[in] targetDic  build target dictionary
# @param[in] platform   platform name
#
# @return (dictionary1, dictionary2)-tupple
#          dictionary1: key=(qtVer, mnemonic, bdType), value=build option list
#          dictionary2: key=(qtVer, mnemonic, bdType), value=log file name
#------------------------------------------------------------------------------
def Get_Build_Options( targetDic, platform ):
    buildOp = dict()
    logfile = dict()

    for qtVer in [5, 6]:
        if qtVer == 5:
            qtType = "Qt5"
        elif qtVer == 6:
            qtType = "Qt6"

        for key in targetDic:
            target = targetDic[key]
            if target == "std":
                buildOp[(qtVer, "std", "r")] = [ '-q', f'{qtType}MacPorts', '-r', 'sys',  '-p', 'sys' ]
                logfile[(qtVer, "std", "r")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "RsysPsys")
                buildOp[(qtVer, "std", "d")] = [ '-q', f'{qtType}MacPorts', '-r', 'sys',  '-p', 'sys', '--debug' ]
                logfile[(qtVer, "std", "d")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "RsysPsys")
            elif target == "ports":
                buildOp[(qtVer, "ports", "r")] = [ '-q', f'{qtType}MacPorts', '-r', 'MP34', '-p', 'MP313' ]
                logfile[(qtVer, "ports", "r")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "Rmp34Pmp313")
                buildOp[(qtVer, "ports", "d")] = [ '-q', f'{qtType}MacPorts', '-r', 'MP34', '-p', 'MP313', '--debug' ]
                logfile[(qtVer, "ports", "d")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "Rmp34Pmp313")
            elif target == "brew":
                buildOp[(qtVer, "brew", "r")] = [ '-q', f'{qtType}Brew', '-r', 'HB34', '-p', 'HB313' ]
                logfile[(qtVer, "brew", "r")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "Rhb34Phb313")
                buildOp[(qtVer, "brew", "d")] = [ '-q', f'{qtType}Brew', '-r', 'HB34', '-p', 'HB313', '--debug' ]
                logfile[(qtVer, "brew", "d")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "Rhb34Phb313")
            elif target == "brewHW":
                buildOp[(qtVer, "brewHW", "r")] = [ '-q', f'{qtType}Brew', '-r', 'sys',  '-p', 'HB311' ]
                logfile[(qtVer, "brewHW", "r")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "RsysPhb311")
                buildOp[(qtVer, "brewHW", "d")] = [ '-q', f'{qtType}Brew', '-r', 'sys',  '-p', 'HB311', '--debug' ]
                logfile[(qtVer, "brewHW", "d")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "RsysPhb311")
            elif target == "ana3":
                buildOp[(qtVer, "ana3", "r")] = [ '-q', f'{qtType}Ana3', '-r', 'Ana3', '-p', 'Ana3' ]
                logfile[(qtVer, "ana3", "r")] = "{}Ana3.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "Rana3Pana3")
                buildOp[(qtVer, "ana3", "d")] = [ '-q', f'{qtType}Ana3', '-r', 'Ana3', '-p', 'Ana3', '--debug' ]
                logfile[(qtVer, "ana3", "d")] = "{}Ana3.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "Rana3Pana3")
            elif target == "brewA":
                buildOp[(qtVer, "brewA", "r")] = [ '-q', f'{qtType}Brew', '-r', 'HB34', '-p', 'HBAuto' ]
                logfile[(qtVer, "brewA", "r")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "Rhb34Phbauto")
                buildOp[(qtVer, "brewA", "d")] = [ '-q', f'{qtType}Brew', '-r', 'HB34', '-p', 'HBAuto', '--debug' ]
                logfile[(qtVer, "brewA", "d")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "Rhb34Phbauto")
            elif target == "brewAHW":
                buildOp[(qtVer, "brewAHW", "r")] = [ '-q', f'{qtType}Brew', '-r', 'sys',  '-p', 'HBAuto' ]
                logfile[(qtVer, "brewAHW", "r")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "RsysPhbauto")
                buildOp[(qtVer, "brewAHW", "d")] = [ '-q', f'{qtType}Brew', '-r', 'sys',  '-p', 'HBAuto', '--debug' ]
                logfile[(qtVer, "brewAHW", "d")] = "{}Brew.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "RsysPhbauto")
            elif target == "pbrew":
                buildOp[(qtVer, "pbrew", "r")] = [ '-q', f'{qtType}MacPorts', '-r', 'HB34', '-p', 'HB313' ]
                logfile[(qtVer, "pbrew", "r")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "Rhb34Phb313")
                buildOp[(qtVer, "pbrew", "d")] = [ '-q', f'{qtType}MacPorts', '-r', 'HB34', '-p', 'HB313', '--debug' ]
                logfile[(qtVer, "pbrew", "d")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "Rhb34Phb313")
            elif target == "pbrewHW":
                buildOp[(qtVer, "pbrewHW", "r")] = [ '-q', f'{qtType}MacPorts', '-r', 'sys',  '-p', 'HB311' ]
                logfile[(qtVer, "pbrewHW", "r")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "release", "RsysPhb311")
                buildOp[(qtVer, "pbrewHW", "d")] = [ '-q', f'{qtType}MacPorts', '-r', 'sys',  '-p', 'HB311', '--debug' ]
                logfile[(qtVer, "pbrewHW", "d")] = "{}MP.build.macos-{}-{}-{}.log".format(qtType.lower(), platform, "debug", "RsysPhb311")

        if WithPymod:
            buildOp[(qtVer, "ports", "r")] = buildOp[(qtVer, "ports", "r")] + ['--buildPymod']
            buildOp[(qtVer, "brew", "r")]  = buildOp[(qtVer, "brew", "r")]
            buildOp[(qtVer, "ana3", "r")]  = buildOp[(qtVer, "ana3", "r")]  + ['--buildPymod']
            buildOp[(qtVer, "pbrew", "r")] = buildOp[(qtVer, "pbrew", "r")]

            buildOp[(qtVer, "ports", "d")] = buildOp[(qtVer, "ports", "d")]
            buildOp[(qtVer, "brew", "d")]  = buildOp[(qtVer, "brew", "d")]
            buildOp[(qtVer, "ana3", "d")]  = buildOp[(qtVer, "ana3", "d")]
            buildOp[(qtVer, "pbrew", "d")] = buildOp[(qtVer, "pbrew", "d")]

    return (buildOp, logfile)

#------------------------------------------------------------------------------
## To get the ".macQAT" dictionary for QA Test
#
# @param[in] targetDic  build target dictionary
# @param[in] platform   platform name
#
# @return a dictionary; key=(qtVer, mnemonic, bdType), value=".macQAT" directory
#------------------------------------------------------------------------------
def Get_QAT_Directory( targetDic, platform ):
    dirQAT = dict()

    for qtVer in [5, 6]:
        if qtVer == 5:
            qtType = "Qt5"
        elif qtVer == 6:
            qtType = "Qt6"

        for key in targetDic:
            target = targetDic[key]
            if target == "std":
                dirQAT[(qtVer, "std", "r")] = f'{qtType.lower()}MP.build.macos-{platform}-release-RsysPsys.macQAT'
                dirQAT[(qtVer, "std", "d")] = f'{qtType.lower()}MP.build.macos-{platform}-debug-RsysPsys.macQAT'
            elif target == "ports":
                dirQAT[(qtVer, "ports", "r")] = f'{qtType.lower()}MP.build.macos-{platform}-release-Rmp34Pmp313.macQAT'
                dirQAT[(qtVer, "ports", "d")] = f'{qtType.lower()}MP.build.macos-{platform}-debug-Rmp34Pmp313.macQAT'
            elif target == "brew":
                dirQAT[(qtVer, "brew", "r")] = f'{qtType.lower()}Brew.build.macos-{platform}-release-Rhb34Phb313.macQAT'
                dirQAT[(qtVer, "brew", "d")] = f'{qtType.lower()}Brew.build.macos-{platform}-debug-Rhb34Phb313.macQAT'
            elif target == "brewHW":
                dirQAT[(qtVer, "brewHW", "r")] = f'{qtType.lower()}Brew.build.macos-{platform}-release-RsysPhb311.macQAT'
                dirQAT[(qtVer, "brewHW", "d")] = f'{qtType.lower()}Brew.build.macos-{platform}-debug-RsysPhb311.macQAT'
            elif target == "ana3":
                dirQAT[(qtVer, "ana3", "r")] = f'{qtType.lower()}Ana3.build.macos-{platform}-release-Rana3Pana3.macQAT'
                dirQAT[(qtVer, "ana3", "d")] = f'{qtType.lower()}Ana3.build.macos-{platform}-debug-Rana3Pana3.macQAT'
            elif target == "brewA":
                dirQAT[(qtVer, "brewA", "r")] = f'{qtType.lower()}Brew.build.macos-{platform}-release-Rhb34Phbauto.macQAT'
                dirQAT[(qtVer, "brewA", "d")] = f'{qtType.lower()}Brew.build.macos-{platform}-debug-Rhb34Phbauto.macQAT'
            elif target == "brewAHW":
                dirQAT[(qtVer, "brewAHW", "r")] = f'{qtType.lower()}Brew.build.macos-{platform}-release-RsysPhbauto.macQAT'
                dirQAT[(qtVer, "brewAHW", "d")] = f'{qtType.lower()}Brew.build.macos-{platform}-debug-RsysPhbauto.macQAT'
            elif target == "pbrew":
                dirQAT[(qtVer, "pbrew", "r")] = f'{qtType.lower()}MP.build.macos-{platform}-release-Rhb34Phb313.macQAT'
                dirQAT[(qtVer, "pbrew", "d")] = f'{qtType.lower()}MP.build.macos-{platform}-debug-Rhb34Phb313.macQAT'
            elif target == "pbrewHW":
                dirQAT[(qtVer, "pbrewHW", "r")] = f'{qtType.lower()}MP.build.macos-{platform}-release-RsysPhb311.macQAT'
                dirQAT[(qtVer, "pbrewHW", "d")] = f'{qtType.lower()}MP.build.macos-{platform}-debug-RsysPhb311.macQAT'

    return dirQAT

#------------------------------------------------------------------------------
## To get the build option dictionary for making/cleaning DMG
#
# @param[in] targetDic  build target dictionary
# @param[in] platform   platform name
# @param[in] srlDMG     serial number of DMG
# @param[in] makeflag   True to make; False to clean
#
# @return a dictionary; key=(qtVer, mnemonic, bdType), value=build option list
#------------------------------------------------------------------------------
def Get_Package_Options( targetDic, platform, srlDMG, makeflag ):
    packOp = dict()

    flag = '-m' if makeflag else '-c'

    for qtVer in [5, 6]:
        if qtVer == 5:
            qtType = "Qt5"
        elif qtVer == 6:
            qtType = "Qt6"

        for key in targetDic:
            target = targetDic[key]
            if target == "std":
                packOp[(qtVer, "std", "r")] = [ '-p', f'ST-{qtType.lower()}MP.pkg.macos-{platform}-release-RsysPsys',
                                                '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "std", "d")] = [ '-p', f'ST-{qtType.lower()}MP.pkg.macos-{platform}-debug-RsysPsys',
                                                '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "ports":
                packOp[(qtVer, "ports", "r")] = [ '-p', f'LW-{qtType.lower()}MP.pkg.macos-{platform}-release-Rmp34Pmp313',
                                                  '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "ports", "d")] = [ '-p', f'LW-{qtType.lower()}MP.pkg.macos-{platform}-debug-Rmp34Pmp313',
                                                  '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "brew":
                packOp[(qtVer, "brew", "r")] = [ '-p', f'LW-{qtType.lower()}Brew.pkg.macos-{platform}-release-Rhb34Phb313',
                                                 '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "brew", "d")] = [ '-p', f'LW-{qtType.lower()}Brew.pkg.macos-{platform}-debug-Rhb34Phb313',
                                                 '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "brewHW":
                packOp[(qtVer, "brewHW", "r")] = [ '-p', f'HW-{qtType.lower()}Brew.pkg.macos-{platform}-release-RsysPhb311',
                                                   '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "brewHW", "d")] = [ '-p', f'HW-{qtType.lower()}Brew.pkg.macos-{platform}-debug-RsysPhb311',
                                                   '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "ana3":
                packOp[(qtVer, "ana3", "r")] = [ '-p', f'LW-{qtType.lower()}Ana3.pkg.macos-{platform}-release-Rana3Pana3',
                                                 '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "ana3", "d")] = [ '-p', f'LW-{qtType.lower()}Ana3.pkg.macos-{platform}-debug-Rana3Pana3',
                                                 '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "brewA":
                packOp[(qtVer, "brewA", "r")] = [ '-p', f'LW-{qtType.lower()}Brew.pkg.macos-{platform}-release-Rhb34Phbauto',
                                                  '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "brewA", "d")] = [ '-p', f'LW-{qtType.lower()}Brew.pkg.macos-{platform}-debug-Rhb34Phbauto',
                                                  '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "brewAHW":
                packOp[(qtVer, "brewAHW", "r")] = [ '-p', f'HW-{qtType.lower()}Brew.pkg.macos-{platform}-release-RsysPhbauto',
                                                    '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "brewAHW", "d")] = [ '-p', f'HW-{qtType.lower()}Brew.pkg.macos-{platform}-debug-RsysPhbauto',
                                                    '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "pbrew":
                packOp[(qtVer, "pbrew", "r")] = [ '-p', f'LW-{qtType.lower()}MP.pkg.macos-{platform}-release-Rhb34Phb313',
                                                  '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "pbrew", "d")] = [ '-p', f'LW-{qtType.lower()}MP.pkg.macos-{platform}-debug-Rhb34Phb313',
                                                  '-s', '%d' % srlDMG, f'{flag}' ]
            elif target == "pbrewHW":
                packOp[(qtVer, "pbrewHW", "r")] = [ '-p', f'HW-{qtType.lower()}MP.pkg.macos-{platform}-release-RsysPhb311',
                                                    '-s', '%d' % srlDMG, f'{flag}' ]
                packOp[(qtVer, "pbrewHW", "d")] = [ '-p', f'HW-{qtType.lower()}MP.pkg.macos-{platform}-debug-RsysPhb311',
                                                    '-s', '%d' % srlDMG, f'{flag}' ]
    return packOp

#------------------------------------------------------------------------------
## To parse the command line arguments
#------------------------------------------------------------------------------
def Parse_CommandLine_Arguments():
    global Usage        # usage
    global QtType       # Qt type
    global Target       # target list
    global QtTarget     # list of (Qt, target, bdType)-tuple
    global Build        # operation flag
    global Deploy       # operation flag
    global WithPymod    # operation flag
    global QATest       # operation flag
    global QACheck      # operation flag
    global MakeDMG      # operation flag
    global CleanDMG     # operation flag
    global Upload       # operation flag
    global SrlDMG       # DMG serial number
    global Dropbox      # Dropbox directory
    global DryRun       # True for dry-run

    platform = Test_My_Platform()
    targetopt = "0,1,2,13,4" if platform in ["Tahoe", "Sequoia", "Sonoma", "Ventura", "Monterey"] else ""

    Usage  = "\n"
    Usage += "----------------------------------------------------------------------------------------------------------\n"
    Usage += " nightlyBuild.py [EXPERIMENTAL]\n"
    Usage += "   << To execute the jobs for making KLayout's DMGs for\n"
    Usage += "                                               macOS Monterey, Ventura, Sonoma, or Sequoia >>\n"
    Usage += "\n"
    Usage += "$ [python] nightlyBuild.py\n"
    Usage += "   option & argument : comment on option if any                              | default value\n"
    Usage += "   --------------------------------------------------------------------------+--------------\n"
    Usage += "   [--qt <type>] : 5='qt5', 6='qt6' (migration to Qt6 is ongoing)            | 5\n"
    Usage += f"   [--target <list>] : 0='std', 1='ports', 2='brew', 3='brewHW', 4='ana3',   | '{targetopt}'\n"
    Usage += "                       5='brewA', 6='brewAHW', 12='pbrew', 13='pbrewHW'      |\n"
    Usage += "                       * with --qt=6, use --target='0,1,2,3' (4 is ignored)  |\n"
    Usage += "   [--qttarget <tuple list>] : ex. '5,1,r'| qt=5, target=1, bdType='r'       | disabled\n"
    Usage += "                                   '5,4,d', '6,4,r', and '6,4,d' are omitted |\n"
    Usage += "      + This option supersedes, if used, the --qt and --target combination.  |\n"
    Usage += "      + You can use this option multiple times.                              |\n"
    Usage += "      + Or you can pass those list by the 'nightlyBuild.csv' file.           |\n"
    Usage += "        A sample file 'macbuild/nightlyBuild.sample.csv' is available.       |\n"
    Usage += "   [--build]  : build and deploy                                             | disabled\n"
    Usage += "   [--deploy] : deploy only                                                  | disabled\n"
    Usage += "   [--pymod]  : build and deploy Pymod, too (release build only)             | disabled\n"
    Usage += "   [--test]   : run the QA Test                                              | disabled\n"
    Usage += "   [--check]  : check the QA Test results                                    | disabled\n"
    Usage += "   [--makedmg|--cleandmg <srlno>] : make or clean DMGs                       | disabled\n"
    Usage += "   [--upload <dropbox>] : upload DMGs to $HOME/Dropbox/klayout/<dropbox>     | disabled\n"
    Usage += "   [--dryrun]           : dry-run for --build option                         | disabled\n"
    Usage += "   [-?|--?]             : print this usage and exit                          | disabled\n"
    Usage += "                                                                             |\n"
    Usage += "      To use this script, make a symbolic link in the project root by:       |\n"
    Usage += "          $ ln -s ./macbuild/nightlyBuild.py .                               |\n"
    Usage += "            + edit and save ./macbuild/nightlyBuild.csv (optional)           |\n"
    Usage += "                                                                             |\n"
    Usage += "      Regular sequence for using this script:                                |\n"
    Usage += "          (1) $ ./nightlyBuild.py  --build  --pymod                          |\n"
    Usage += "          (2)   (confirm the build results)                                  |\n"
    Usage += "          (3) $ ./nightlyBuild.py  --test                                    |\n"
    Usage += "          (4) $ ./nightlyBuild.py  --check (confirm the QA Test results)     |\n"
    Usage += "          (5) $ ./nightlyBuild.py  --makedmg  1                              |\n"
    Usage += "          (6) $ ./nightlyBuild.py  --upload  '0.30.2'                        |\n"
    Usage += "          (7) $ ./nightlyBuild.py  --cleandmg 1                              |\n"
    Usage += "-----------------------------------------------------------------------------+----------------------------\n"

    p = optparse.OptionParser( usage=Usage )
    p.add_option( '--qt',
                    dest='qt_type',
                    help='Qt5 or Qt6 (5)' )

    p.add_option( '--target',
                    dest='targets',
                    help='build target list' )

    p.add_option( '--qttarget',
                    action='append',
                    dest='qt_target',
                    help='(Qt, target, bdType)-tuple' )

    p.add_option( '--build',
                    action='store_true',
                    dest='build',
                    default=False,
                    help='build and deploy' )

    p.add_option( '--deploy',
                    action='store_true',
                    dest='deploy',
                    default=False,
                    help='deploy only' )

    p.add_option( '--pymod',
                    action='store_true',
                    dest='with_pymod',
                    default=False,
                    help='build and deploy Pymod, too ' )

    p.add_option( '--test',
                    action='store_true',
                    dest='qa_test',
                    default=False,
                    help='run the QA Test' )

    p.add_option( '--check',
                    action='store_true',
                    dest='qa_check',
                    default=False,
                    help='check the QA Test results' )

    p.add_option( '--makedmg',
                    dest='makedmg',
                    help='make DMG' )

    p.add_option( '--cleandmg',
                    dest='cleandmg',
                    help='clean DMG' )

    p.add_option( '--upload',
                    dest='upload',
                    help='upload to Dropbox' )

    p.add_option( '--dryrun',
                    action='store_true',
                    dest='dry_run',
                    default=False,
                    help='dry-run' )

    p.add_option( '-?', '--??',
                    action='store_true',
                    dest='checkusage',
                    default=False,
                    help='check usage' )

    p.set_defaults( qt_type    = "5",
                    targets    = f"{targetopt}",
                    qt_target  = list(),
                    build      = False,
                    deploy     = False,
                    with_pymod = False,
                    qa_test    = False,
                    qa_check   = False,
                    makedmg    = "",
                    cleandmg   = "",
                    upload     = "",
                    dry_run    = False,
                    checkusage = False )

    opt, args = p.parse_args()
    if opt.checkusage:
        print(Usage)
        sys.exit(0)

    myPlatform = Test_My_Platform( [ 'Monterey', 'Ventura', 'Sonoma', 'Sequoia', 'Tahoe' ] )
    if myPlatform == "":
        print( "! Current platform is not [ 'Monterey', 'Ventura', 'Sonoma', 'Sequoia', 'Tahoe' ]" )
        print(Usage)
        sys.exit(0)

    QtType = int(opt.qt_type)
    if QtType not in [5, 6]:
        print( "! Invalid Qt type <%d>" % QtType )
        print(Usage)
        sys.exit(0)

    targetIdx = list()
    raw       = (opt.targets or "").strip()
    targets   = [int(item) for item in raw.split(",") if item.strip() != ""]
    print(targets)
    if len(targets) != 0:
        for target in targets:
            if target not in targetIdx:
                targetIdx.append(target)  # first appeared and non-duplicated index

    targetDic = Get_Build_Target_Dict()
    Target    = list()
    for idx in targetIdx:
        if idx in [0,1,2,3,4,5,6,12,13]:
            Target.append( targetDic[idx] )

    # Populate QtTarget
    QtTarget = list()
    for target in Target:
        QtTarget.append( (QtType, target, 'r') )
    QtType = None
    Target = None
    print( "# The --qt and --target combination specifies for 'r'elease build..." )
    print(QtTarget)

    if len(opt.qt_target) == 1 and opt.qt_target[0] == "nightlyBuild.csv": # reserved file name
        QtTarget     = list()
        withqttarget = True
        df = pd.read_csv( opt.qt_target[0], comment="#" )
        if len(df) == 0:
            print( "! --qttarget==nightlyBuild.csv is used but DataFrame is empty" )
            print(Usage)
            sys.exit(0)
        for i in range(0, len(df)):
            qt     = df.iloc[i,0]
            idx    = df.iloc[i,1]
            bdType = df.iloc[i,2].lower()[0]
            if (qt == 5 and idx in [0,1,2,3,4,5,6,12,13] and bdType in ['r']) or \
               (qt == 5 and idx in [0,1,2,3,  5,6,12,13] and bdType in ['d']) or \
               (qt == 6 and idx in [0,1,2,3,4,5,6,12,13] and bdType in ['r', 'd']):
                QtTarget.append( (qt, targetDic[idx], bdType) )
    elif len(opt.qt_target) > 0:
        QtTarget     = list()
        withqttarget = True
        for item in opt.qt_target:
            qt     = int(item.split(",")[0])
            idx    = int(item.split(",")[1])
            bdType =    (item.split(",")[2]).lower()[0]
            if (qt == 5 and idx in [0,1,2,3,4,5,6,12,13] and bdType in ['r']) or \
               (qt == 5 and idx in [0,1,2,3,  5,6,12,13] and bdType in ['d']) or \
               (qt == 6 and idx in [0,1,2,3,4,5,6,12,13] and bdType in ['r', 'd']):
                QtTarget.append( (qt, targetDic[idx], bdType) )
    else:
        withqttarget = False

    if withqttarget:
        if len(QtTarget) > 0:
            print( "# The --qttarget option superseded the --qt and --target combination" )
            print(QtTarget)
        else:
            print( "! --qttarget is used but there is no valid (Qt, target, bdTye)-tuple" )
            print(Usage)
            sys.exit(0)

    Build     = opt.build
    Deploy    = opt.deploy
    WithPymod = opt.with_pymod
    QATest    = opt.qa_test
    QACheck   = opt.qa_check
    MakeDMG   = False
    CleanDMG  = False
    Upload    = False
    DryRun    = opt.dry_run

    if opt.makedmg != "":
        MakeDMG = True
        SrlDMG  = int(opt.makedmg)

    if opt.cleandmg != "":
        CleanDMG = True
        SrlDMG   = int(opt.cleandmg)

    if MakeDMG and CleanDMG:
        print( "! --makedmg and --cleandmg cannot be used simultaneously" )
        print(Usage)
        sys.exit(0)

    if opt.upload != "":
        Upload  = True
        Dropbox = opt.upload

    if not (Build or Deploy or QATest or QACheck or MakeDMG or CleanDMG or Upload):
        print( "! No action selected" )
        print(Usage)
        sys.exit(0)

#------------------------------------------------------------------------------
## To build and deploy
#------------------------------------------------------------------------------
def Build_Deploy( deployonly=False ):
    pyBuilder  = "./build4mac.py"
    myPlatform = Test_My_Platform()
    buildOp, logfile = Get_Build_Options( Get_Build_Target_Dict(), myPlatform )

    for qttype, key, bdType in QtTarget:
        if key == "ana3" and bdType == 'd': # anaconda3 does not provide debug_lib
            continue

        deplog = logfile[(qttype, key, bdType)].replace( ".log", ".dep.log" )

        command1 = [ pyBuilder ] + buildOp[(qttype, key, bdType)]

        if key in [ "std", "brewHW", "brewAHW", "pbrewHW" ] :
            command2  = "time"
            command2 += f" \\\n  {pyBuilder}"
            for option in buildOp[(qttype, key, bdType)]:
                command2 += f" \\\n  {option}"
            command2 += " \\\n  {}".format('-y')
            command2 += f"  2>&1 | tee {deplog}; \\\n"
            command2 += "test ${PIPESTATUS[0]} -eq 0"  # tee always exits with 0
        else:
            command2  = "time"
            command2 += f" \\\n  {pyBuilder}"
            for option in buildOp[(qttype, key, bdType)]:
                command2 += f" \\\n  {option}"
            command2 += " \\\n  {}".format('-Y')
            command2 += f"  2>&1 | tee {deplog}; \\\n"
            command2 += "test ${PIPESTATUS[0]} -eq 0"  # tee always exits with 0

        if DryRun:
            print( f"### Target = <{key}> ###" )
            print(command1)
            print(command2)
            print( "" )
            continue

        if not deployonly:
            if subprocess.call( command1, shell=False ) != 0:
                print( "", file=sys.stderr )
                print( "-----------------------------------------------------------------", file=sys.stderr )
                print( f"!!! <{pyBuilder}>: failed to build KLayout", file=sys.stderr )
                print( "-----------------------------------------------------------------", file=sys.stderr )
                print( "", file=sys.stderr )
                sys.exit(1)
            else:
                print( "", file=sys.stderr )
                print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
                print( f"### <{pyBuilder}>: successfully built KLayout", file=sys.stderr )
                print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
                print( "", file=sys.stderr )

        if subprocess.call( command2, shell=True ) != 0:
            print( "", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( f"!!! <{pyBuilder}>: failed to deploy KLayout", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( "", file=sys.stderr )
            sys.exit(1)
        else:
            print( "", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( f"### <{pyBuilder}>: successfully deployed KLayout", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( "", file=sys.stderr )

#------------------------------------------------------------------------------
## To run the QA tests
#
# @param[in] excludeList    list of tests to exclude such as ['pymod', 'pya']
#------------------------------------------------------------------------------
def Run_QATest( excludeList ):
    pyRunnerQAT = "./macQAT.py"
    myPlatform  = Test_My_Platform()
    dirQAT      = Get_QAT_Directory( Get_Build_Target_Dict(), myPlatform )

    for qttype, key, bdType in QtTarget:
        if key == "ana3" and bdType == 'd': # anaconda3 does not provide debug_lib
            continue

        if key == "ana3":
            excludeList += ['pymod']
        exclude = ",".join( sorted( set(excludeList) ) )

        command1 = [ pyRunnerQAT ] + [ '--run' ]
        if exclude != "":
            command1 += [ '--exclude', f'{exclude}' ]
        print( dirQAT[(qttype, key, bdType)], command1 )
        #continue
        os.chdir( dirQAT[(qttype, key, bdType)] )

        if subprocess.call( command1, shell=False ) != 0:
            print( "", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( f"!!! <{pyRunnerQAT}>: failed to run the QA Test", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( "", file=sys.stderr )
            sys.exit(1)
        else:
            print( "", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( f"### <{pyRunnerQAT}>: successfully ran the QA Test", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( "", file=sys.stderr )

        os.chdir( "../" )

#------------------------------------------------------------------------------
## To check the QA test results
#
# @param[in] lines  number of lines to dump from the tail
#------------------------------------------------------------------------------
def Check_QATest_Results( lines ):
    tailCommand = "/usr/bin/tail"
    myPlatform  = Test_My_Platform()
    dirQAT      = Get_QAT_Directory( Get_Build_Target_Dict(), myPlatform )

    for qttype, key, bdType in QtTarget:
        if key == "ana3" and bdType == 'd': # anaconda3 does not provide debug_lib
            continue

        os.chdir( dirQAT[(qttype, key, bdType)] )
        logfile  = glob.glob( "*.log" )

        if not logfile:
            print( "", file=sys.stderr )
            print( f"[skip] No *.log files found in '{dirQAT[(qttype, key, bdType)]}'", file=sys.stderr )
            print( "", file=sys.stderr )
            os.chdir("../")
            continue

        command1 = [ tailCommand ] + [ '-n', '%d' % lines ] + logfile
        print( dirQAT[(qttype, key, bdType)], command1 )
        #continue

        if subprocess.call( command1, shell=False ) != 0:
            print( "", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( f"!!! <{tailCommand}>: failed to check the QA Test results", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( "", file=sys.stderr )
            sys.exit(1)
        else:
            print( "", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( f"### <{tailCommand}>: successfully checked the QA Test results", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( "", file=sys.stderr )

        os.chdir( "../" )

#------------------------------------------------------------------------------
## To make DMGs
#
# @param[in] srlDMG     DMG's serial number
#------------------------------------------------------------------------------
def DMG_Make( srlDMG ):
    pyDMGmaker = "./makeDMG4mac.py"
    stashDMG   = "./DMGStash"
    myPlatform = Test_My_Platform()
    packOp     = Get_Package_Options( Get_Build_Target_Dict(), myPlatform, srlDMG, makeflag=True )

    if os.path.isdir( stashDMG ):
        shutil.rmtree( stashDMG )
    os.mkdir( stashDMG )

    for qttype, key, bdType in QtTarget:
        if key == "ana3" and bdType == 'd': # anaconda3 does not provide debug_lib
            continue

        command1 = [ pyDMGmaker ] + packOp[(qttype, key, bdType)]
        print(command1)
        #continue

        if subprocess.call( command1, shell=False ) != 0:
            print( "", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( f"!!! <{pyDMGmaker}>: failed to make KLayout DMG", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( "", file=sys.stderr )
            sys.exit(1)
        else:
            print( "", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( f"### <{pyDMGmaker}>: successfully made KLayout DMG", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( "", file=sys.stderr )

        dmgs = glob.glob( "*.dmg*" )
        for item in dmgs:
            shutil.move( item, stashDMG )

#------------------------------------------------------------------------------
## To clean up DMGs
#
# @param[in] srlDMG     DMG's serial number
#------------------------------------------------------------------------------
def DMG_Clean( srlDMG ):
    pyDMGmaker = "./makeDMG4mac.py"
    stashDMG   = "./DMGStash"
    myPlatform = Test_My_Platform()
    packOp     = Get_Package_Options( Get_Build_Target_Dict(), myPlatform, srlDMG, makeflag=False )

    if os.path.isdir( stashDMG ):
        shutil.rmtree( stashDMG )

    for qttype, key, bdType in QtTarget:
        if key == "ana3" and bdType == 'd': # anaconda3 does not provide debug_lib
            continue

        command1 = [ pyDMGmaker ] + packOp[(qttype, key, bdType)]
        print(command1)
        #continue

        if subprocess.call( command1, shell=False ) != 0:
            print( "", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( f"!!! <{pyDMGmaker}>: failed to clean KLayout DMG", file=sys.stderr )
            print( "-----------------------------------------------------------------", file=sys.stderr )
            print( "", file=sys.stderr )
            sys.exit(1)
        else:
            print( "", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( f"### <{pyDMGmaker}>: successfully cleaned KLayout DMG", file=sys.stderr )
            print( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", file=sys.stderr )
            print( "", file=sys.stderr )

#------------------------------------------------------------------------------
## To upload DMGs to Dropbox
#
# @param[in] targetdir     existing target directory such as "0.26.9"
#------------------------------------------------------------------------------
def Upload_To_Dropbox( targetdir ):
    stashDMG = "./DMGStash"

    distDir = os.environ["HOME"] + "/Dropbox/klayout/" + targetdir
    if not os.path.isdir(distDir):
        os.makedirs(distDir)

    dmgs = glob.glob( f"{stashDMG}/*.dmg*" )
    for item in dmgs:
        shutil.copy2( item, distDir )

#------------------------------------------------------------------------------
## The main function
#------------------------------------------------------------------------------
def Main():
    Parse_CommandLine_Arguments()

    if Build:
        Build_Deploy(deployonly=False)
    if Deploy:
        Build_Deploy(deployonly=True)
        sys.exit(0)
    if QATest:
        Run_QATest( [] ) # ex. ['pymod', 'pya']
    if QACheck:
        Check_QATest_Results( 20 )
    elif MakeDMG:
        DMG_Make( SrlDMG )
    elif Upload:
        Upload_To_Dropbox( Dropbox )
    elif CleanDMG:
        DMG_Clean( SrlDMG )

#===================================================================================
if __name__ == "__main__":
    Main()

#---------------
# End of file
#---------------
