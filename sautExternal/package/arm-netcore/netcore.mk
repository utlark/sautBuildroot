################################################################################
#
# armnetcore
#
################################################################################

ARM_NETCORE_VERSION = 3.1.27
ARM_NETCORE_SITE = https://dotnet.microsoft.com
ARM_NETCORE_SOURCE = /download/dotnet-core/thank-you/runtime-aspnetcore-$(ARM_NETCORE_VERSION)-linux-arm32-binaries

define ARM_NETCORE_EXTRACT_CMDS
	cat $(ARM_NETCORE_DL_DIR)/runtime-aspnetcore-$(ARM_NETCORE_VERSION)-linux-arm32-binaries | grep "window.open" | awk -F"\"" '{print $$2}' | xargs wget -N -P $(ARM_NETCORE_DL_DIR)
	tar -zxf $(ARM_NETCORE_DL_DIR)/*.tar.gz -C $(@D)
endef

define ARM_NETCORE_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/microsoft
	cp -a $(@D)/* $(TARGET_DIR)/usr/share/microsoft/
	ln -fs ../share/microsoft/dotnet $(TARGET_DIR)/usr/bin/dotnet 
	ln -fs ./libdl.so.2 $(TARGET_DIR)/lib/libdl.so 
	echo "export DOTNET_ROOT=/usr/share/microsoft" > $(TARGET_DIR)/etc/profile.d/dotnet.sh
endef

$(eval $(generic-package))
