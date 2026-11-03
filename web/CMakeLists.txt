mirante_add_module(web
    API       0.1
    SOURCES   WebModule.cpp WebUrls.cpp WebUrls.h
    RESOURCES icons/web.svg
    TESTS     tests/tst_from_url.cpp tests/tst_register.cpp tests/tst_translations.cpp tests/tst_urls.cpp
    DEFAULT   ON)
