zephyr_library()

zephyr_library_sources(paj7620.c)
zephyr_library_sources_ifdef(CONFIG_PAJ7620_TRIGGER paj7620_trigger.c)
