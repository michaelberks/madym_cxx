#-------------------------------------------------------------------------
#Install python wrappers

#First run version script to generate python version file
install(CODE
        "execute_process(
            COMMAND ${CMAKE_SOURCE_DIR}/python/version.sh 
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
            )"
        COMPONENT Python
        CONFIGURATIONS Release)

#Now copy over the python files from source to deploy
install(DIRECTORY "${CMAKE_SOURCE_DIR}/python" DESTINATION "${MADYM_DEPLOY_DIR}"
    USE_SOURCE_PERMISSIONS
    COMPONENT Python
    CONFIGURATIONS Release
    PATTERN "build" EXCLUDE
    PATTERN "dist" EXCLUDE
    PATTERN "__pycache__" EXCLUDE
    PATTERN "*.egg-info" EXCLUDE
    )