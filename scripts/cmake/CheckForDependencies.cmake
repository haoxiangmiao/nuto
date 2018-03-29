macro(check_for_dependencies)
    include(CheckBoost)

    #find_package(MUMPS QUIET)
    #if(MUMPS_FOUND)
    #    message(STATUS "MUMPS solver found.")
    #else()
    #    warning("MUMPS solver not found - will not be available.")
    #endif()

    find_package(PARDISO QUIET)
    if(PARDISO_FOUND)
        message(STATUS "Pardsio solver found.")
    else()
        warning("Pardiso solver not found - will not be available.")
    endif()

    # find Eigen header files (Linear Algebra)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)
    message(STATUS "Eigen version = ${EIGEN3_VERSION_STRING}")

    # find ARPACK library for eigenvalue analysis
    find_package(ARPACK QUIET)
    if(ARPACK_FOUND)
        message(STATUS "Arpack solver found.")
    else()
        warning("Arpack not found. EigenSolverArpack will not be available.")
    endif()

    find_package(benchmark QUIET)
    if(benchmark_FOUND)
        message(STATUS "Found google benchmark")
    else()
        warning("Google benchmark NOT found - Benchmarks won't be build!")
    endif()

    find_package(SUITESPARSE QUIET)
    if(SUITESPARSE_FOUND)
        message(STATUS "Found SuiteSparse solver library")
    else()
        warning("SuiteSparse NOT found - certain solvers won't be available")
    endif()


    find_package(MKL)
    if(MKL_FOUND)
        message(STATUS "Found Intel MKL")
        
    endif()

endmacro()
