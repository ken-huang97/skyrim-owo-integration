#pragma once
#include <iostream>

class OWOException : public std::exception
{
public:
    virtual char* GetCode() = 0;
};

class NetworkError : OWOException
{
public:
    char* GetCode() { return (char*)"A network error ocurred"; }
};

class WrongSensationFormatException : OWOException
{
public:
    char* GetCode() { return (char*)"Sensation was not in a correct format"; }
};

class WrongMusclesFormatException : OWOException
{
public:
    char* GetCode() { return (char*)"Muscle was not in a correct format"; }
};

class WrongGameAuthFormatException : OWOException
{
public:
    char* GetCode() { return (char*)"GameAuth was not in a correct format"; }
};