$(build_dir)$(POKEGRA_FOOTPRINTS_ARC)/%.bin : $(POKEGRA_FOOTPRINTS_ROOT)/%.bin
	@ mkdir -p $(@D)
	cp $^ $@