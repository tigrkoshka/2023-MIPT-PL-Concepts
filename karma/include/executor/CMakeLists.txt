add_library(
        executor OBJECT
        executor.cpp
        impl.cpp
        j_executor.cpp
        ri_executor.cpp
        rr_executor.cpp
        rm_executor.cpp
        common_executor.cpp
        executor_base.cpp
        storage.cpp
        config.cpp
        errors.cpp
)
