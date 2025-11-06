.PHONY:           \
	all           \
	clean         \
	configure     \
	distclean     \
	meson         \
	purge         \
	release       \
	rom           \
	update

SUBPROJ_DIR := subprojects

MESON_VER := 1.7.0
MESON_DIR := $(SUBPROJ_DIR)/meson-$(MESON_VER)
MESON_SUB := $(MESON_DIR)/meson.py

MESON ?= $(MESON_SUB)
NINJA ?= ninja
GIT ?= git

BUILD ?= build

all: release

.NOTPARALLEL: release
release: rom

.NOTPARALLEL: rom

rom: $(BUILD)/build.ninja data
	$(NINJA) -C $(BUILD) White2Upgrade.nds

clean: $(BUILD)/build.ninja
	$(MESON) compile -C $(BUILD) --clean

distclean:
	rm -rf $(BUILD)

purge: distclean
	rm -rf $(SKREW_DIR)
ifeq ($(MESON),$(MESON_SUB))
	! test -f $(MESON) || $(MESON) subprojects purge --confirm
	rm -rf $(MESON_DIR)
else
	$(MESON) subprojects purge --confirm
endif

configure: $(BUILD)/build.ninja

$(BUILD)/build.ninja: meson
	$(MESON) setup $(BUILD) --cross-file=meson/nitro.ini

$(BUILD):
	mkdir -p -- $(BUILD)

meson: ;
ifeq ($(MESON),$(MESON_SUB))
meson: $(MESON_SUB)
endif

$(MESON_SUB):
	$(GIT) clone --depth=1 -b $(MESON_VER) https://github.com/mesonbuild/meson $(@D)
