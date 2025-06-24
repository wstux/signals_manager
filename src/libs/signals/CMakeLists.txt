LibTarget(signals STATIC
    SOURCES
        details/manager.cpp
        details/utils.cpp
    COMPILE_DEFINITIONS
        ${SIGNALS_MANAGER_USE_BOOST_LOCKFREE}
    DEPENDS
        ${boost}
)

