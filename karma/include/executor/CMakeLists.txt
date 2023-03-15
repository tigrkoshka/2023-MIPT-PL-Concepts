add_library(
        executor OBJECT
        executor.cpp
        impl.cpp
        j_executor.cpp
        ri_executor.cpp
        rr_executor.cpp
        rm_executor.cpp
        common_executor.cpp
        command_executor.cpp
        storage.cpp
        config.cpp
        errors.cpp
)
