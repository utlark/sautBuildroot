################################################################################
#
# saut-qmlnet
#
################################################################################

SAUT_QMLNET_VERSION = saut/from-master
SAUT_QMLNET_SITE = https://github.com/NpoSaut/qmlnet.git
SAUT_QMLNET_SITE_METHOD = git
SAUT_QMLNET_DEPENDENCIES = qt5quickcontrols2
SAUT_QMLNET_GIT_SUBMODULES = YES

define SAUT_QMLNET_BUILD_CMDS
	mkdir -p $(@D)/src/native/build
	$(HOST_DIR)/usr/bin/qmake -o $(@D)/src/native/build/Makefile $(@D)/src/native/QmlNet/QmlNet.pro
	$(MAKE) -C $(@D)/src/native/build
endef

define SAUT_QMLNET_INSTALL_TARGET_CMDS
	cp -f $(@D)/src/native/build/libQmlNet.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
