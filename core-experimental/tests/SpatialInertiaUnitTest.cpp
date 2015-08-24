/*
 * Copyright (C) 2015 Fondazione Istituto Italiano di Tecnologia
 * Authors: Silvio Traversaro
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include <iDynTree/Core/EigenHelpers.h>
#include <iDynTree/Core/IMatrix.h>
#include <iDynTree/Core/SpatialInertia.h>
#include <iDynTree/Core/SpatialMomentum.h>
#include <iDynTree/Core/Transform.h>
#include <iDynTree/Core/Utils.h>
#include <iDynTree/Core/TestUtils.h>
#include <iDynTree/Core/Twist.h>
#include <iDynTree/Core/MatrixFixSize.h>

#include <Eigen/Dense>

#include <cstdio>
#include <cstdlib>

using namespace iDynTree;

void checkInertiaTransformation(const Transform & trans, const SpatialInertia & inertia)
{
    SpatialInertia inertiaTranslated = trans*inertia;
    Matrix6x6      inertiaTranslatedCheck;

    Matrix6x6 adjWre = trans.asAdjointTransformWrench();
    Matrix6x6 I      = inertia.asMatrix();
    Matrix6x6 adj    = trans.inverse().asAdjointTransform();
    toEigen(inertiaTranslatedCheck) = toEigen(adjWre)*
                                      toEigen(I)*
                                      toEigen(adj);

    Matrix6x6 inertiaTranslatedRaw = inertiaTranslated.asMatrix();
    ASSERT_EQUAL_MATRIX(inertiaTranslatedCheck,inertiaTranslatedRaw);
}

void checkInertiaTwistProduct(const SpatialInertia & inertia, const Twist & twist)
{
    SpatialMomentum momentum = inertia*twist;
    Vector6         momentumCheck;

    Matrix6x6 I = inertia.asMatrix();

    toEigen(momentumCheck) = toEigen(I)*toEigen(twist);

    ASSERT_EQUAL_VECTOR(momentum,momentumCheck);
}

void checkInvariance(const Transform & trans, const SpatialInertia & inertia, const Twist & twist)
{
    Transform invTrans = trans.inverse();
    SpatialMomentum momentumTranslated = trans*(inertia*twist);
    SpatialMomentum momentumTranslatedCheck = (trans*inertia)*(trans*twist);

    Twist           twistTranslated         = trans*twist;
    SpatialInertia  inertiaTranslated       = trans*inertia;
    Vector6 momentumTranslatedCheck2;
    Vector6 momentumTranslatedCheck3;
    Vector6 twistTranslatedCheck;
    Matrix6x6 transAdjWrench = trans.asAdjointTransformWrench();
    Matrix6x6 inertiaRaw     = inertia.asMatrix();
    Matrix6x6 transInvAdj    = trans.inverse().asAdjointTransform();
    Matrix6x6 transAdj       = trans.asAdjointTransform();
    Matrix6x6 inertiaTranslatedCheck;

    toEigen(momentumTranslatedCheck2) = toEigen(transAdjWrench)*
                                        toEigen(inertiaRaw)*
                                        toEigen(twist);

    toEigen(momentumTranslatedCheck3) = toEigen(transAdjWrench)*
                                        toEigen(inertiaRaw)*
                                        toEigen(transInvAdj)*
                                        toEigen(transAdj)*
                                        toEigen(twist);

    toEigen(twistTranslatedCheck)     = toEigen(transAdj)*
                                        toEigen(twist);

    toEigen(inertiaTranslatedCheck)   = toEigen(transAdjWrench)*
                                        toEigen(inertiaRaw)*
                                        toEigen(transInvAdj);

    ASSERT_EQUAL_MATRIX(inertiaTranslatedCheck,inertiaTranslated.asMatrix());
    ASSERT_EQUAL_VECTOR(twistTranslated,twistTranslatedCheck);
    ASSERT_EQUAL_VECTOR(momentumTranslated,momentumTranslatedCheck3);
    ASSERT_EQUAL_VECTOR(momentumTranslated,momentumTranslatedCheck2);
    ASSERT_EQUAL_VECTOR(momentumTranslated,momentumTranslatedCheck);

    SpatialMomentum momentum = invTrans*momentumTranslated;
    SpatialMomentum momentumCheck = (invTrans*(trans*inertia))*(invTrans*(trans*twist));

    ASSERT_EQUAL_VECTOR(momentum,momentumCheck);
}

int main()
{
    Transform trans(Rotation::RPY(0.0,0.0,0.0),Position(10,0,0));

    double twistData[6] = {0.0,0.0,0.0,1.0,2.0,3.0};
    Twist twist(twistData,6);

    ASSERT_EQUAL_DOUBLE(twist(0),twistData[0]);

    double rotInertiaData[3*3] = {10.0,0.0,0.0,
                                  0.0,20.0,0.0,
                                  0.0,0.0,30.0};
    SpatialInertia inertia(1.0,Position(100,0,0),RotationalInertiaRaw(rotInertiaData,3,3));

    checkInertiaTwistProduct(inertia,twist);
    checkInertiaTransformation(trans,inertia);
    checkInvariance(trans,inertia,twist);

    return EXIT_SUCCESS;
}