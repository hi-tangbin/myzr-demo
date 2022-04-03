# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lte_dial
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Fibocom
  TITLE:=fibocom lte dial
  MAINTAINER:=zhagyang
  DEPENDS :=+libuci
endef

define Package/$(PKG_NAME)/description
 fibocom 5G gobinet dial
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef


# $(CP) ./files/* $(1)/
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) ./files/lte_dial $(1)/etc/init.d/lte_dial
	$(INSTALL_BIN) ./files/lte_dial.conf $(1)/etc/config/lte_dial
	$(INSTALL_BIN) ./files/module_info $(1)/etc/config/module_info
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
