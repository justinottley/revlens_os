
#ifndef CORE_PYMODULE_H
#define CORE_PYMODULE_H

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "RlpCore/PYMODULE/vigenere.h"


class CryptWrapper {


public:
    CryptWrapper();

    py::str encrypt(const char* strIn)
    {
        std::string sIn(strIn);
        std::string enMsg = VIG_encrypt(sIn, _key);

        py::str result = enMsg;
        return result;
    }

    py::str decrypt(const char* encIn)
    {
        std::string sIn(encIn);
        std::string decMsg = VIG_decrypt(sIn, _key);

        py::str result = decMsg;
        return result;
    }


private:
    std::string _key;
};



void RlpCorePYMODULE_bootstrap();

#endif