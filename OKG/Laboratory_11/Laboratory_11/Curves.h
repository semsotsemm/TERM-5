#pragma once

#include "CMatrix.h"

double Lagr(CMatrix& X, CMatrix& Y, double x);
void Bezier(CMatrix& X, CMatrix& Y, CMatrix& XB, CMatrix& YB, int M);
