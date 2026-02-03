# DEPRECATED
# this script is not used anymore
# use lipo_all.py
#
scons targets=helloworld platform_target=iOS-armv7-8.0
scons targets=helloworld platform_target=iOS-arm64-8.0

echo "running lipo.."
lipo -create /Users/justinottley/dev/revlens_root/sp_revlens/inst/iOS-arm64-8.0/helloworld/global/lib/libRlpHelloworldGUI.a /Users/justinottley/dev/revlens_root/sp_revlens/inst/iOS-armv7-8.0/helloworld/global/lib/libRlpHelloworldGUI.a -output /Users/justinottley/dev/revlens_root/sp_revlens/inst/iOS-8.0/helloworld/global/lib/libRlpHelloworldGUI.a