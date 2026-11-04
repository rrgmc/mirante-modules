mirante_add_module(system
    API       0.1
    SOURCES   ResourcesView.cpp ResourcesView.h SystemModule.cpp SystemResources.cpp SystemResources.h
    RESOURCES icons/system.svg
    TESTS     tests/tst_register.cpp tests/tst_resources.cpp tests/tst_view.cpp
    DEFAULT   ON)
