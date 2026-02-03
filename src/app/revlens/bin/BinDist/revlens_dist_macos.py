#!/usr/bin/env python3


import os
import sys
import argparse
import datetime
import subprocess

ERROR_PREFIX_NOT_FOUND = 1

mydir = os.path.dirname(__file__)

src = '/Users/justinottley/dev/revlens_root/revlens_public/inst/Darwin'
tb = '/Users/justinottley/dev/revlens_root/thirdbase/25_04/Darwin-arm64'
tbFull = '/Users/justinottley/dev/revlens_root/thirdbase/25_04/Darwin-arm64-26.2'

prefix = '/Users/justinottley/dev/revlens_root/dist_macos'

appDir = os.path.join(prefix, 'Revlens.app')
contentsDir = os.path.join(appDir, 'Contents')

DEV_ROOT = '/Users/justinottley/dev/revlens_root/revlens_public'
PYTHON_FRAMEWORK_PATH = '/Library/Frameworks/Python.framework'

QT_VERSION = '6.8.3'
PY_VERSION = '3.12'

SIGN = 'Developer ID Application: Justin Ottley (4GZG5XZ9A2)'


def distThirdbaseLibs(prefix):

    libDest = os.path.join(contentsDir, 'lib')
    qnpSrc = os.path.join(tb, 'qnanopainter', QT_VERSION, 'macos', 'lib')

    cmd = 'cp {}/libqnanopainter.1.dylib {}'.format(qnpSrc, libDest)
    print(cmd)
    os.system(cmd)


    qnpSrc = os.path.join(tb, 'qtmypaint', QT_VERSION, 'macos', 'lib')
    cmd = 'cp {}/libmypaint.1.dylib {}'.format(qnpSrc, libDest)
    print(cmd)
    os.system(cmd)


    pylibsSrc = os.path.join(tb, 'pylibs')
    if os.path.isdir(pylibsSrc):
        cmd = 'cp -R {} {}'.format(pylibsSrc, libDest)
        print(cmd)
        os.system(cmd)

    otioSrc = os.path.join(tb, 'OpenTimelineIO/25_10_py311')
    if not os.path.isdir(otioSrc):
        print('OTIO NOT FOUND')
        return


    otioDest = os.path.join(libDest, 'OpenTimelineIO')
    if not os.path.isdir(otioDest):
        os.mkdir(otioDest)

    cmd = 'cp -R {} {}/OpenTimelineIO'.format(otioSrc, libDest)
    print(cmd)
    os.system(cmd)

    otioExModDir = '{}/OpenTimelineIO/25_10_py311/lib/python{}/site-packages/opentimelineio'.format(libDest, PY_VERSION)
    for fileEntry in os.listdir(otioExModDir):
        if not fileEntry.endswith('.so'):
            continue
        
        filePath = os.path.join(otioExModDir, fileEntry)
        cmd = 'otool -L {}'.format(filePath)

        result = subprocess.check_output(cmd, shell=True).decode('utf-8')
        for line in result.split('\n'):
            line = line.strip()
            if not line:
                continue
            
            lineParts = line.split()
            libPart = lineParts[0]

            if '@loader_path' in libPart:

                newEntry = libPart.replace('@loader_path', '@rpath/OpenTimelineIO/25_10_py311/lib')
                cmd = 'install_name_tool -change {} {} {}'.format(libPart, newEntry, filePath)
                print(cmd)
                os.system(cmd)


def distRevlensLibs(src, prefix):

    for ssetDir in ['core', 'gui', 'edb', 'revlens', 'extrevlens', 'prod']:

        ssetTopDir = os.path.join(src, ssetDir)
        for libEntry in os.listdir(os.path.join(ssetTopDir, 'lib')):
            print('Checking {} for dylibs'.format(libEntry))
            if not libEntry.endswith('.dylib'):
                continue

            dylibPath = os.path.join(ssetTopDir, 'lib', libEntry)
            destDylibPath = os.path.join(contentsDir, 'lib', libEntry)
            cmd = 'cp {} {}'.format(dylibPath, destDylibPath)
            print(cmd)
            os.system(cmd)

            fixRevlensLibPath(destDylibPath)
            fixPythonLibPath(destDylibPath)
            fixThirdbaseLibPaths(destDylibPath)
            fixFFmpegLibPaths(destDylibPath)


    for ssetDir in ['core', 'gui', 'edb', 'revlens', 'extrevlens', 'prod']:

        ssetTopDir = os.path.join(src, ssetDir)
        ssetDestTopDir = os.path.join(contentsDir, 'lib', 'inst', 'Darwin', ssetDir)

        for dirEntry in ['bin', 'lib', 'etc', 'include']:
            ssetSrcDir = os.path.join(ssetTopDir, dirEntry)

            ssetDestDir = os.path.join(ssetDestTopDir, dirEntry)
            if not os.path.isdir(ssetDestDir):
                print('Creating {}'.format(ssetDestDir))
                os.makedirs(ssetDestDir)

            if os.path.isdir(ssetSrcDir):
                cmd = 'cp -R {}/ {}'.format(ssetSrcDir, ssetDestDir)
                print(cmd)
                os.system(cmd)

        # ARGH.. remove dylibs, we copied them into a flat location instead earlier
        for dirEntry in ['lib', 'bin']:
            for root, dirs, files in os.walk(os.path.join(ssetDestTopDir, dirEntry)):
                for fileEntry in files:
                    if fileEntry.endswith('.dylib') or fileEntry.endswith('.binexec'):
                        fileExecPath = os.path.join(root, fileEntry)
                        print('Removing from {} tree: {}'.format(dirEntry, fileExecPath))
                        os.remove(fileExecPath)



def distAppBundle():


    if not os.path.isdir(prefix):
        print('Error: dir not found: {}'.format(prefix))
        sys.exit(ERROR_PREFIX_NOT_FOUND)

    appDir = 'Revlens.app'

    appDirPath = os.path.join(prefix, appDir)
    appContentsPath = os.path.join(appDirPath, 'Contents')


    allDirs = [appDirPath, appContentsPath]

    # Frameworks, include
    for subdir in ['lib', 'MacOS', 'PlugIns', 'Resources', 'Frameworks']: #  'PlugIns/Qt'
        allDirs.append(os.path.join(appContentsPath, subdir))


    for appDir in allDirs:
        if not os.path.isdir(appDir):
            print(appDir)
            os.mkdir(appDir)


    infoPListSrc = os.path.join(mydir, 'Info.plist')
    iconSrc = os.path.join(mydir, 'revlens_logo_only_2025.icns')
    # qtConfSrc = os.path.join(mydir, 'qt.conf.macos')

    resourcesDir = os.path.join(appContentsPath, 'Resources')
    cmd = 'cp {} {}'.format(infoPListSrc, appContentsPath)
    print(cmd)
    os.system(cmd)

    cmd = 'cp {} {}'.format(iconSrc, resourcesDir)
    print(cmd)
    os.system(cmd)

    # cmd = 'cp {} {}/qt.conf'.format(qtConfSrc, resourcesDir)
    # print(cmd)
    # os.system(cmd)


def distQtBad(prefix):

    # /Users/justinottley/dev/revlens_root/thirdbase/25_04/Darwin-arm64/Qt/6.8.3/macos
    excludes = [
        '.DS_Store',
        'bin/',
        '*/*dSYM',
        '*.a',
        '*.prl',
        # '*.app',
        # 'metatypes/',
        # 'qml/',
        'sbom/',
        # 'plugins/permissions/',
        # 'libexec/'
    ]


    cmd = 'rsync {}'.format(''.join([' --exclude {}'.format(exc) for exc in excludes]))
    # print(cmd)

    cmd = '{} -avz --progress {}/Qt/{}/macos/ {}/Revlens.app/Contents/PlugIns/Qt'.format(
        cmd, tb, QT_VERSION, prefix)
    print(cmd)
    os.system(cmd)


def distQt(prefix):

    mdqt = '{}/Qt/{}/macos/bin/macdeployqt'.format(tb, QT_VERSION)
    appPath = './Revlens.app' # .format(prefix)
    qmldir = '{}/src/gui/rlp_gui/static/icons/misc'.format(DEV_ROOT)
    cmd = '{} {} -qmldir={} -verbose=1'.format(
        mdqt, appPath, qmldir
    )

    print(cmd)
    os.chdir(prefix)
    os.system(cmd)



def distPython(prefix):

    print('Dist Python')

    cmd = 'cp -R /Library/Frameworks/Python.framework {}/Frameworks'.format(contentsDir)
    print(cmd)
    os.system(cmd)

    pyVersionDir = os.path.join(contentsDir, 'Frameworks', 'Python.framework', 'Versions', PY_VERSION)

    mainLibPath = os.path.join(pyVersionDir, 'Python')
    assert(os.path.isfile(mainLibPath))

    cmd = 'install_name_tool -id @rpath/Frameworks/Versions/{}/Python {}'.format(PY_VERSION, mainLibPath)
    print(cmd)
    os.system(cmd)


    for dylib in os.listdir(os.path.join(pyVersionDir, 'lib')):
        if not dylib.endswith('.dylib'):
            continue

        libPath = os.path.join(pyVersionDir, 'lib', dylib)
        fixPythonLibPath(libPath)


    for pySo in os.listdir(os.path.join(pyVersionDir, 'lib', 'python{}'.format(PY_VERSION), 'lib-dynload')):
        if not pySo.endswith('.so'):
            continue

        soPath = os.path.join(pyVersionDir, 'lib', 'python{}'.format(PY_VERSION), 'lib-dynload', pySo)
        fixPythonLibPath(soPath, fixid=False)


    pyAppExec1 = '{}/Frameworks/Python.framework/Resources/Python.app/Contents/MacOS/Python'.format(contentsDir)
    fixPythonLibPath(pyAppExec1, fixid=False, addRpath='@executable_path/../../../../../../../../Frameworks')

    pyAppExec2 = '{}/Frameworks/Python.framework/Versions/{}/bin/python{}'.format(contentsDir, PY_VERSION, PY_VERSION)
    fixPythonLibPath(pyAppExec2, fixid=False, addRpath='@executable_path/../../../../../Frameworks')



def fixPythonLibPath(libPath, fixid=True, addRpath=None):

    print('Fix Python lib: {}'.format(libPath))

    otoolLibCmd = 'otool -L {}'.format(libPath)
    result = subprocess.check_output(otoolLibCmd, shell=True)
    result = result.decode('utf-8')
    for line in result.split('\n'):
        line = line.strip()
        if not line:
            continue

        lineParts = line.split()
        libPart = lineParts[0]

        if PYTHON_FRAMEWORK_PATH in line:
            # print('Found Python lib: {}'.format(libPart))
            newLibPath = libPart.replace(PYTHON_FRAMEWORK_PATH, '@rpath/Python.framework')

            cmd = 'install_name_tool -change {} {} {}'.format(libPart, newLibPath, libPath)
            print(cmd)
            os.system(cmd)

    if fixid:
        libFilename = os.path.basename(libPath)
        cmd = 'install_name_tool -id @rpath/Python.framework/Versions/{}/lib/{} {}'.format(
            PY_VERSION, libFilename, libPath)
        print(cmd)
        os.system(cmd)


    if addRpath:
        cmd = 'install_name_tool -add_rpath {} {}'.format(addRpath, libPath)
        print(cmd)
        os.system(cmd)



def fixRevlensLibPath(libPath, fixid=True):

    otoolLibCmd = 'otool -L {}'.format(libPath)
    result = subprocess.check_output(otoolLibCmd, shell=True)
    result = result.decode('utf-8')
    for line in result.split('\n'):
        line = line.strip()
        if not line:
            continue

        lineParts = line.split()
        libPart = lineParts[0]

        if 'libRlp' in line:
            print('Found RLP lib: {}'.format(libPart))
            libName = os.path.basename(libPart)
            newLibPath = '@rpath/{}'.format(libName)
            cmd = 'install_name_tool -change {} {} {}'.format(libPart, newLibPath, libPath)
            print(cmd)
            os.system(cmd)


    if fixid:
        fixIdCmd = 'install_name_tool -id @rpath/{} {}'.format(os.path.basename(libPath), libPath)
        print(fixIdCmd)
        os.system(fixIdCmd)


def fixThirdbaseLibPaths(libPath):

    tbIdentifiers = [
        'libmypaint',
        'libqnanopainter'
    ]

    otoolLibCmd = 'otool -L {}'.format(libPath)
    result = subprocess.check_output(otoolLibCmd, shell=True)
    result = result.decode('utf-8')
    for line in result.split('\n'):
        line = line.strip()
        if not line:
            continue

        lineParts = line.split()
        libPart = lineParts[0]

        for tbIdentifier in tbIdentifiers:
            if tbIdentifier in line:
                print('Found thirdbase lib: {}'.format(libPart))
                libName = os.path.basename(libPart)
                newLibPath = '@rpath/{}'.format(libName)
                cmd = 'install_name_tool -change {} {} {}'.format(libPart, newLibPath, libPath)
                print(cmd)
                os.system(cmd)



def fixQtPaths(prefix):

    execDest = os.path.join(contentsDir, 'MacOS')

    qtLibPath = '{}/Qt/{}/macos/lib'.format(tb, QT_VERSION)
    qtfullLibPath = '{}/Qt/{}/macos/lib'.format(tbFull, QT_VERSION)

    for root, dirs, files in os.walk(os.path.join(contentsDir, 'lib')):

        allFiles = []
        if '.framework/Versions/A' in root and len(files) == 1:
            allFiles.append(os.path.join(root, files[0]))


        for fileEntry in files:

            if fileEntry.endswith('.dylib') or fileEntry.endswith('.so'):
            
                filePath = os.path.join(root, fileEntry)
                allFiles.append(filePath)


        for filePath in allFiles:

                otoolCmd = 'otool -L {}'.format(filePath)
                result = subprocess.check_output(otoolCmd, shell=True).decode('utf-8')
                for line in result.split('\n'):
                    line = line.strip()
                    if not line:
                        continue
                    lineParts = line.split()
                    libPart = lineParts[0].strip()

                    newQtLib = None
                    if qtLibPath in libPart:
                        newQtLib = libPart.replace(qtLibPath, '@rpath')

                    elif qtfullLibPath in libPart:
                        newQtLib = libPart.replace(qtfullLibPath, '@rpath')

                    if newQtLib:
                        cmd = 'install_name_tool -change {} {} {}'.format(
                            libPart, newQtLib, filePath
                        )
                        print(cmd)
                        os.system(cmd)


                otoolCmd = 'otool -l {}'.format(filePath)
                # print(otoolCmd)

                result = subprocess.check_output(otoolCmd, shell=True).decode('utf-8')

                for line in result.split('\n'):
                    line = line.strip()
                    if not line:
                        continue

                    lineParts = line.split()
                    if lineParts[0] == 'path':

                        # print(line)
                        if '6.8.3' in lineParts[1]:
                            print('FOUND {}'.format(filePath))
                            rpathEntry = lineParts[1]
                            cmd = 'install_name_tool -delete_rpath {} {}'.format(rpathEntry, filePath)
                            print(cmd)
                            os.system(cmd)


                        rpathEntry = lineParts[1]
                        cmd = 'install_name_tool -delete_rpath {} {}/rlp_gui'.format(rpathEntry, execDest)
                        # print(cmd)
                        # os.system(cmd)




def distExec(prefix):

    execSrc = os.path.join(src, 'prod', 'bin', 'rlp_gui')
    execDest = os.path.join(contentsDir, 'MacOS', 'Revlens')

    cmd = 'cp {} {}'.format(execSrc, execDest)
    print(cmd)
    os.system(cmd)

    fixPythonLibPath(execDest)
    fixFFmpegLibPaths(execDest)
    fixRevlensLibPath(execDest, fixid=False)


    otoolCmd = 'otool -l {}'.format(execDest)
    result = subprocess.check_output(otoolCmd, shell=True)
    result = result.decode('utf-8')
    for line in result.split('\n'):
        line = line.strip()
        if not line:
            continue

        lineParts = line.split()
        libPart = lineParts[0]
        if libPart == 'path':

            # print(line)
            if DEV_ROOT in lineParts[1]:
                rpathEntry = lineParts[1]
                cmd = 'install_name_tool -delete_rpath {} {}'.format(rpathEntry, execDest)
                print(cmd)
                os.system(cmd)

            if '6.8.3' in lineParts[1]:
                rpathEntry = lineParts[1]
                cmd = 'install_name_tool -delete_rpath {} {}'.format(rpathEntry, execDest)
                print(cmd)
                os.system(cmd)


    # print(result)


    for libPath in [
        '@executable_path/../lib',
        '@executable_path/../Frameworks',
        # '@executable_path/../PlugIns/Qt/lib',
        # '@executable_path/../Frameworks/Python.framework/Versions/{}/lib'.format(PY_VERSION)
    ]:

        cmd = 'install_name_tool -add_rpath {} {}'.format(libPath, execDest)
        print(cmd)
        os.system(cmd)


    # cmd = 'cp /Users/justinottley/dev/revlens_root/revlens_public/src/app/revlens/iOS_XCode/codesign_entitlements.xml {}/Resources/revlens.entitlements'.format(contentsDir)
    # print(cmd)
    # os.system(cmd)


def fixFFmpegLibPaths(execPath):

    libDir = '{}/Revlens.app/Contents/lib'.format(prefix)

    # ffTbDir = '{}/FFmpeg/7.1/macos/lib'
    print('Fixing ffmpeg: {}'.format(execPath))

    cmd = 'otool -L {}'.format(execPath)
    result = subprocess.check_output(cmd, shell=True).decode('utf-8')
    for line in result.split('\n'):
        line = line.strip()
        if not line:
            continue

        lineParts = line.split()
        # print(lineParts)
        libPart = lineParts[0]

        # print(libPart)
        if 'ffmpeg' in libPart:

            fflibname = os.path.basename(libPart)
            newname = '@rpath/{}'.format(fflibname)

            ffDestLibPath = os.path.join(libDir, fflibname)

            cmd = 'install_name_tool -change {} {} {}'.format(libPart, newname, execPath)
            print(cmd)
            os.system(cmd)

            if not os.path.isfile(ffDestLibPath):
                print('Copying ffmpeg lib: {}'.format(fflibname))
                cpCmd = 'cp {} {}'.format(libPart,ffDestLibPath)
                print(cpCmd)
                os.system(cpCmd)

                fixFFmpegLibPaths(ffDestLibPath)

                cmd = 'install_name_tool -id @rpath/{} {}'.format(fflibname, ffDestLibPath)
                print('Fixing id: {} -> {}'.format(fflibname, cmd))
                os.system(cmd)




def codeSignLibs(prefix):

    libDir = os.path.join(contentsDir, 'lib')
    for fileEntry in os.listdir(libDir):
        if not fileEntry.endswith('.dylib'):
            continue

        filePath = os.path.join(libDir, fileEntry)
        codesignCmd = "codesign --timestamp --force --sign '{}' {}".format(SIGN, filePath)
        print(codesignCmd)
        os.system(codesignCmd)

    removePaths = []
    for root, dirs, files in os.walk(os.path.join(contentsDir, 'lib', 'inst', 'Darwin')):
        for fileEntry in files:
            if fileEntry.endswith('.so'):
                filePath = os.path.join(root, fileEntry)
                codesignCmd = "codesign --timestamp --force --sign '{}' {}".format(SIGN, filePath)
                print(codesignCmd)
                os.system(codesignCmd)

            if '/bin/' in root:

                if fileEntry == 'rlp_gui':
                    removePaths.append(os.path.join(root, fileEntry))

                else:
                    filePath = os.path.join(root, fileEntry)
                    codesignCmd = "codesign --timestamp --force --sign '{}' --options runtime {}".format(SIGN, filePath)
                    print(codesignCmd)
                    os.system(codesignCmd)


    for removePath in removePaths:
        print('Removing {}'.format(removePath))
        os.remove(removePath)


def codeSignPython(prefix):
    
    pyRoot = os.path.join(contentsDir, 'Frameworks', 'Python.framework')


    print('Code signing Python: {}'.format(pyRoot))

    pylibDir = os.path.join(pyRoot, 'Versions', PY_VERSION, 'lib')

    for root, dirs, files in os.walk(pylibDir):

        for fileEntry in files:
            if fileEntry.endswith('.dylib') or fileEntry.endswith('.so') or fileEntry.endswith('.o'):
                filePath = os.path.join(root, fileEntry)
                codesignCmd = "codesign --timestamp --force --sign '{}' {}".format(SIGN, filePath)
                print(codesignCmd)
                os.system(codesignCmd)

    


    codesignCmd = "codesign --timestamp --force --sign '{}' {}/Python".format(SIGN, pyRoot)
    print(codesignCmd)
    os.system(codesignCmd)


    for execEntry in [
        '{}/Versions/{}/bin/python{}'.format(pyRoot, PY_VERSION, PY_VERSION),
        '{}/Resources/Python.app/Contents/MacOS/Python'.format(pyRoot),
        '{}/Resources/Python.app'.format(pyRoot),
        pyRoot
    ]:
        codesignCmd = "codesign --timestamp --force --sign '{}' --options runtime {}".format(SIGN, execEntry)
        print(codesignCmd)
        os.system(codesignCmd)



def codeSignQt(prefix):
    qtRoot = os.path.join(contentsDir, 'Frameworks')
    print('Code signing Qt libs in: {}'.format(qtRoot))
    for framework in os.listdir(qtRoot):
        frameworkName = framework.replace('.framework', '')
        frameworkLibName = os.path.join(qtRoot, framework, frameworkName)
        if os.path.isfile(frameworkLibName):
            codesignCmd = "codesign --timestamp --force --sign '{}' {}".format(SIGN, frameworkLibName)
            print(codesignCmd)
            os.system(codesignCmd)

        frameworkPath = os.path.join(qtRoot, framework)
        codesignCmd = "codesign --timestamp --force --sign '{}' --options runtime {}".format(SIGN, frameworkPath)
        print(codesignCmd)
        os.system(codesignCmd)

    print('')

    # codesign QtWebEngineProcess with hardened runtime

    weProcessPath = os.path.join(contentsDir, 'PlugIns', 'Qt', 'lib', 'QtWebEngineCore.framework', 'Helpers', 'QtWebEngineProcess.app')
    if os.path.isdir(weProcessPath):
        codesignCmd = "codesign --timestamp --force --sign '{}' --options runtime {}".format(SIGN, weProcessPath)
        print(codesignCmd)
        os.system(codesignCmd)


    pluginsDir = os.path.join(contentsDir, 'PlugIns')
    for root, dirs, files in os.walk(pluginsDir):
        for fileEntry in files:
            filePath = os.path.join(root, fileEntry)
            
            codesignCmd = "codesign --timestamp --force --sign '{}' {}".format(SIGN, filePath)
            print(codesignCmd)
            os.system(codesignCmd)


def codeSignExec(prefix):

    # clean extended attributes, fixes codesign problems
    cmd = 'xattr -cr {}/Revlens.app'.format(prefix)
    print(cmd)
    os.system(cmd)


    signCmd = "codesign --force --options runtime --sign '{}' {}/Revlens.app/Contents/MacOS/Revlens".format(SIGN, prefix)
    print(signCmd)
    os.system(signCmd)


    # signCmd = "codesign --force --options runtime --entitlements ./src/app/revlens/iOS_XCode/codesign_entitlements.xml --sign '{}' {}/Revlens.app".format(SIGN, prefix)
    signCmd = "codesign --force --options runtime --sign '{}' {}/Revlens.app".format(SIGN, prefix)
    
    print(signCmd)
    os.system(signCmd)



def createDmg(prefix):

    today = datetime.datetime.today()
    timesuffix = '{}_{}_{}'.format(today.year, str(today.month).zfill(2), str(today.day).zfill(2))

    cmd = 'hdiutil create -srcfolder {}/Revlens.app -volname "Revlens {}" -fs HFS+ -format UDZO {}/Revlens_{}.dmg'.format(
        prefix, timesuffix, prefix, timesuffix)

    print(cmd)
    os.system(cmd)

def notarize(prefix):

    today = datetime.datetime.today()
    timesuffix = '{}_{}_{}'.format(today.year, str(today.month).zfill(2), str(today.day).zfill(2))

    os.chdir(prefix)
    cmd = 'xcrun notarytool submit Revlens_{}.dmg --apple-id justin.ottley@gmail.com --team-id 4GZG5XZ9A2 --password gmud-xwfs-tiyz-maar --wait'.format(
        timesuffix
    )
    print(cmd)

    os.system(cmd)

def getNotarizeLog(logId):

    cmd = 'xcrun notarytool log {} --password gmud-xwfs-tiyz-maar --apple-id justin.ottley@gmail.com --team-id 4GZG5XZ9A'.format(
        logId
    )
    print(cmd)
    os.system(cmd)


def main():

    parser = argparse.ArgumentParser('')
    parser.add_argument('--dist', action='store_true', default=False)
    parser.add_argument('--distqt', action='store_true', default=False)
    parser.add_argument('--sign', action='store_true', default=False)
    parser.add_argument('--dmg', action='store_true', default=False)
    parser.add_argument('--notarize', action='store_true', default=False)
    parser.add_argument('--log', required=False)


    args = parser.parse_args()

    if args.dist:
        distAppBundle()
        distThirdbaseLibs(prefix)
        distPython(prefix)
        distRevlensLibs(src, prefix)
        distExec(prefix)

        distQt(prefix)

    if args.distqt:
         distAppBundle()
         distExec(prefix)
         distQt(prefix)


    if args.sign:
        codeSignLibs(prefix)
        codeSignQt(prefix)
        codeSignPython(prefix)
        codeSignExec(prefix)

    if args.dmg:
        createDmg(prefix)

    if args.notarize:
        notarize(prefix)

    if args.log:
        getNotarizeLog(args.log)

    print('')
    print('----')
    print('')


if __name__ == '__main__':
    main()