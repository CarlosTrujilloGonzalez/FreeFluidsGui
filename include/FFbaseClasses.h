/*
 * FFbaseClasses.h
 *
 *  Created on: 23/04/2017
 *      Author: Carlos Trujillo
 *
 *This file is part of the "Free Fluids" application
 *Copyright (C) 2008-2018  Carlos Trujillo Gonzalez

 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License version 3
 *as published by the Free Software Foundation

 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.

 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "FFbasic.h"
#include "FFeosMix.h"
#include<string>
#include<vector>
#include<iterator>
#ifndef FFBASECLASSES
#define FFBASECLASSES

namespace FF {

//Class Substance
//---------------
class Substance
{
public:
    Substance();
    //~Substance();

    int id;
     FF_BaseProp baseProp;
    double MW,Tc,Pc,Vc,Zc,w,Zra,mu,VdWV;

    enum FF_EOS eos;
    enum FF_EosType eosType;
     FF_CubicEOSdata cubicData;
     FF_SaftEOSdata saftData;
     FF_SWEOSdata swData;

     FF_Correlation cp0Corr;
     FF_Correlation vpCorr;
     FF_Correlation hVsatCorr;
     FF_Correlation lCpCorr;
     FF_Correlation lDensCorr;
     FF_Correlation lViscCorr;
     FF_Correlation lThCCorr;
     FF_Correlation lSurfTCorr;
     FF_Correlation gDensCorr;
     FF_Correlation gViscCorr;
     FF_Correlation gThCCorr;
     FF_Correlation sDensCorr;
     FF_Correlation sCpCorr;
};


//Class ThermoSystem
//------------------
class ThermoSystem {
public:
    ThermoSystem();
    ThermoSystem(int num);
    ThermoSystem(int num, Substance substance[]);
    ~ThermoSystem();
    //Adds a substance and modifies the interaction arrays, enlarging them if necessary
    void AddSubstance(Substance *subs);
    //Deletes a substance by its id
    void DeleteSubstanceByPosition(int pos);
    //Modifies a substance in the given position, putting to 0 the interaction parameters
    void ModifySubstanceByPosition(int pos, Substance *subs);
    //Modifies a set of eos interaction parameters, identified by the substances positions
    void ModifyIntParamEos(int i,int j,double val[6]);
    void MixVfromTPeos(double *T, double *P,double c[],char *option,double resultL[],double resultG[],char *state);
    void MixFF_IdealThermoEOS(double c[], FF_ThermoProperties *th);
    void MixThermoEOS(double c[], FF_ThermoProperties *th);
    void BubbleT(const double *P,const double c[],const double *bTguess,double *bT,double y[],double subsPhiL[],double subsPhiG[]);
    void DewT(const double *P,const double c[],const double *dTguess,double *dT,double x[],double subsPhiL[],double subsPhiG[]);
    void BubbleP(const double *T,const double c[],const double *bPguess,double *bP,double y[],double subsPhiL[],double subsPhiG[]);
    void DewP(const double *T,const double c[],const double *dPguess,double *dP,double x[],double subsPhiL[],double subsPhiG[]);
    void PhiFromActivity(bool *useVp,double *T,double *P,double c[],double gamma[],double phi[],double *gE);

     FF_BaseProp *baseProp;
    enum FF_EosType eosType;
     FF_CubicEOSdata *cubicData=NULL;
     FF_SaftEOSdata *saftData=NULL;
     FF_SWEOSdata *swData=NULL;
    enum FF_MixingRule rule=FF_NoMixRul;
    double *pintParEos=NULL;

    enum FF_IntParamForm actIntForm;
    enum FF_ActModel actModel=FF_NoModel;
    double *pintParAct=NULL;

    double refT;
    double refP;

     FF_Correlation *cp0Corr=NULL;
     FF_Correlation *vpCorr=NULL;
     FF_Correlation *hVsatCorr=NULL;
     FF_Correlation *lCpCorr=NULL;
     FF_Correlation *lDensCorr=NULL;
     FF_Correlation *lViscCorr=NULL;
     FF_Correlation *lThCCorr=NULL;
     FF_Correlation *lSurfTCorr=NULL;
     FF_Correlation *gDensCorr=NULL;
     FF_Correlation *gViscCorr=NULL;
     FF_Correlation *gThCCorr=NULL;
     FF_Correlation *sDensCorr=NULL;
     FF_Correlation *sCpCorr=NULL;

    int numMax;
    int numSubs;
};

class ThermoSystem2 {
public:
    ThermoSystem2();
    ThermoSystem2(int num);
    ThermoSystem2(int num, Substance substance[]);
    ~ThermoSystem2();
    //Adds a substance and modifies the interaction arrays, enlarging them if necessary
    void AddSubstance(Substance *subs);
    //Deletes a substance by its id
    void DeleteSubstanceById(int id);
    //Deletes a substance by its position and modifies the interaction arrays, without shrinking them
    void DeleteSubstanceByPosition(int pos);
    //Modifies a substance in the given position, putting to 0 the interaction parameters
    void ModifySubstanceByPosition(int pos, Substance *subs);
    //Modifies a set of eos interaction parameters, identified by the substances positions
    void ModifyIntParamEos(int i,int j,double val[6]);

    void MixVfromTPeos(double *T, double *P,double c[],char *option,double resultL[],double resultG[],char *state);
    void MixFF_IdealThermoEOS(double c[],double *refT,double *refP, FF_ThermoProperties *th);
    void MixThermoEOS(double c[],double *refT,double *refP, FF_ThermoProperties *th);
    void PhiFromActivity(bool *useVp,double *T,double *P,double c[],double gamma[],double phi[],double *gE);

    int numMax;
    int numSubs;
    std::vector <Substance> substance;
    enum FF_EosType eosType=FF_NoType;
    enum FF_MixingRule rule=FF_NoMixRul;
    double *pintParEos=NULL;

    enum FF_ActModel actModel=FF_NoModel;
    double *pintParAct=NULL;

    double refT=298.15;
    double refP=101325;


};

}//end FF

#endif // FFBASECLASSES

