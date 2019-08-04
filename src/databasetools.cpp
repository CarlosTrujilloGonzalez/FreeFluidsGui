/*
 * databasetools.cpp
 *
 *  Created on: 9/11/2015
 *      Author: Carlos Trujillo
 *
 *This file is part of the "Free Fluids" application
 *Copyright (C) 2008-2019  Carlos Trujillo Gonzalez

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

#include "databasetools.h"

#include <QtSql/QSqlDatabase>
#include <QtSql>

#include <math.h>
#include <stdio.h>
#include <iostream>

//EOS conversion from Qstring to enumeration
void ConvertEosToEnumeration(const QString *eosModel,enum FF_EOS *eos)
{
    if (*eosModel=="PCSAFT") *eos=FF_PCSAFT;
    else if (*eosModel=="PPCSAFT_GV") *eos=FF_PPCSAFT_GV;
    else if (*eosModel=="PPCSAFT_JC") *eos=FF_PPCSAFT_JC;
    else if (*eosModel=="PCSAFTPOL1") *eos=FF_PCSAFTPOL1;
    else if (*eosModel=="SAFTVRMie") *eos=FF_SAFTVRMie;
    else if (*eosModel=="PSAFTVRMie_GV") *eos=FF_PSAFTVRMie_GV;
    else if (*eosModel=="PSAFTVRMie_JC") *eos=FF_PSAFTVRMie_JC;
    else if (*eosModel=="SW") *eos=FF_SW;
    else if (*eosModel=="IAPWS95") *eos=FF_IAPWS95;
    else if (*eosModel=="PR76") *eos=FF_PR76;
    else if (*eosModel=="PR78") *eos=FF_PR78;
    else if (*eosModel=="PRvTWU91") *eos=FF_PRvTWU91;
    else if (*eosModel=="PRSV1") *eos=FF_PRSV1;
    else if (*eosModel=="PRBM") *eos=FF_PRBM;
    else if (*eosModel=="PRMELHEM") *eos=FF_PRMELHEM;
    else if (*eosModel=="PRSOF") *eos=FF_PRSOF;
    else if (*eosModel=="PRALMEIDA") *eos=FF_PRALMEIDA;
    else if (*eosModel=="PRMC") *eos=FF_PRMC;
    else if (*eosModel=="PRTWU91") *eos=FF_PRTWU91;
    else if (*eosModel=="PRTWU95") *eos=FF_PRTWU95;
    else if (*eosModel=="PRPOL1") *eos=FF_PRPOL1;
    else if (*eosModel=="PRFIT3") *eos=FF_PRFIT3;
    else if (*eosModel=="PRFIT4") *eos=FF_PRFIT4;
    else if (*eosModel=="PRFIT4B") *eos=FF_PRFIT4B;
    else if (*eosModel=="SRK") *eos=FF_SRK;
    else if (*eosModel=="SRKSOF") *eos=FF_SRKSOF;
    else if (*eosModel=="SRKMC") *eos=FF_SRKMC;
    else if (*eosModel=="SRKTWU91") *eos=FF_SRKTWU91;
    else if (*eosModel=="SRKPOL2") *eos=FF_SRKPOL2;
}


//EOS conversion from std::string to enumeration
void ConvertEosToEnumeration2(const std::string *eosModel,enum FF_EOS *eos)
{
    if (*eosModel=="PCSAFT") *eos=FF_PCSAFT;
    else if (*eosModel=="PPCSAFT_GV") *eos=FF_PPCSAFT_GV;
    else if (*eosModel=="PPCSAFT_JC") *eos=FF_PPCSAFT_JC;
    else if (*eosModel=="PCSAFTPOL1") *eos=FF_PCSAFTPOL1;
    else if (*eosModel=="SAFTVRMie") *eos=FF_SAFTVRMie;
    else if (*eosModel=="PSAFTVRMie_GV") *eos=FF_PSAFTVRMie_GV;
    else if (*eosModel=="PSAFTVRMie_JC") *eos=FF_PSAFTVRMie_JC;
    else if (*eosModel=="SW") *eos=FF_SW;
    else if (*eosModel=="IAPWS95") *eos=FF_IAPWS95;
    else if (*eosModel=="PR76") *eos=FF_PR76;
    else if (*eosModel=="PR78") *eos=FF_PR78;
    else if (*eosModel=="PRvTWU91") *eos=FF_PRvTWU91;
    else if (*eosModel=="PRSV1") *eos=FF_PRSV1;
    else if (*eosModel=="PRBM") *eos=FF_PRBM;
    else if (*eosModel=="PRMELHEM") *eos=FF_PRMELHEM;
    else if (*eosModel=="PRSOF") *eos=FF_PRSOF;
    else if (*eosModel=="PRALMEIDA") *eos=FF_PRALMEIDA;
    else if (*eosModel=="PRMC") *eos=FF_PRMC;
    else if (*eosModel=="PRTWU91") *eos=FF_PRTWU91;
    else if (*eosModel=="PRTWU95") *eos=FF_PRTWU95;
    else if (*eosModel=="PRPOL1") *eos=FF_PRPOL1;
    else if (*eosModel=="PRFIT3") *eos=FF_PRFIT3;
    else if (*eosModel=="PRFIT4") *eos=FF_PRFIT4;
    else if (*eosModel=="PRFIT4B") *eos=FF_PRFIT4B;
    else if (*eosModel=="SRK") *eos=FF_SRK;
    else if (*eosModel=="SRKSOF") *eos=FF_SRKSOF;
    else if (*eosModel=="SRKMC") *eos=FF_SRKMC;
    else if (*eosModel=="SRKTWU91") *eos=FF_SRKTWU91;
    else if (*eosModel=="SRKPOL2") *eos=FF_SRKPOL2;
}

//EOS conversion from enumeration to std::string
void ConvertEnumerationToEos(const enum FF_EOS *eos, QString *eosModel)
{
    if (*eos==FF_PCSAFT) *eosModel="PCSAFT";
    else if (*eos==FF_PCSAFT1A) *eosModel="PCSAFT";
    else if (*eos==FF_PCSAFT2A) *eosModel="PCSAFT";
    else if (*eos==FF_PCSAFT2B) *eosModel="PCSAFT";
    else if (*eos==FF_PCSAFT3B) *eosModel="PCSAFT";
    else if (*eos==FF_PCSAFT4C) *eosModel="PCSAFT";
    else if (*eos==FF_PPCSAFT_GV) *eosModel="PPCSAFT_GV";
    else if (*eos==FF_PPCSAFT_JC) *eosModel="PPCSAFT_JC";
    else if (*eos==FF_PPCSAFT1A_GV) *eosModel="PPCSAFT_GV";
    else if (*eos==FF_PPCSAFT2B_GV) *eosModel="PPCSAFT_GV";
    else if (*eos==FF_PPCSAFT2B_JC) *eosModel="PPCSAFT_JC";
    else if (*eos==FF_PPCSAFT3B_GV) *eosModel="PPCSAFT_GV";
    else if (*eos==FF_PPCSAFT4C_GV) *eosModel="PPCSAFT_GV";
    else if (*eos==FF_PCSAFTPOL1) *eosModel="PCSAFTPOL1";
    else if (*eos==FF_SAFTVRMie) *eosModel="SAFTVRMie";
    else if (*eos==FF_SAFTVRMie1A) *eosModel="SAFTVRMie";
    else if (*eos==FF_SAFTVRMie2B) *eosModel="SAFTVRMie";
    else if (*eos==FF_SAFTVRMie4C) *eosModel="SAFTVRMie";
    else if (*eos==FF_PSAFTVRMie_GV) *eosModel="PSAFTVRMie_GV";
    else if (*eos==FF_PSAFTVRMie_JC) *eosModel="PSAFTVRMie_JC";
    else if (*eos==FF_SW) *eosModel="SW";
    else if (*eos==FF_IAPWS95) *eosModel="IAPWS95";
    else if (*eos==FF_PR76) *eosModel="PR76";
    else if (*eos==FF_PR78) *eosModel="PR78";
    else if (*eos==FF_PRvTWU91) *eosModel="PRvTWU91";
    else if (*eos==FF_PRSV1) *eosModel="PRSV1";
    else if (*eos==FF_PRBM) *eosModel="PRBM";
    else if (*eos==FF_PRMELHEM) *eosModel="PRMELHEM";
    else if (*eos==FF_PRSOF) *eosModel="PRSOF";
    else if (*eos==FF_PRALMEIDA) *eosModel="PRALMEIDA";
    else if (*eos==FF_PRMC) *eosModel="PRMC";
    else if (*eos==FF_PRTWU91) *eosModel="PRTWU91";
    else if (*eos==FF_PRTWU95) *eosModel="PRTWU95";
    else if (*eos==FF_PRPOL1) *eosModel="PRPOL1";
    else if (*eos==FF_PRFIT3) *eosModel="PRFIT3";
    else if (*eos==FF_PRFIT4) *eosModel="PRFIT4";
    else if (*eos==FF_PRFIT4B) *eosModel="PRFIT4B";
    else if (*eos==FF_SRK) *eosModel="SRK";
    else if (*eos==FF_SRKSOF) *eosModel="SRKSOF";
    else if (*eos==FF_SRKMC) *eosModel="SRKMC";
    else if (*eos==FF_SRKTWU91) *eosModel="SRKTWU91";
    else if (*eos==FF_SRKPOL2) *eosModel="SRKPOL2";
}

//Get basic data for a SubstanceData structure from the database. This means baseProp, Unifac composition and puntual physical properties
void GetBasicData(int id,FF_SubstanceData *subsData,QSqlDatabase *db){
    QSqlQuery query1(*db);
    int i;
    //We query the Products table for general data
    query1.prepare("SELECT * FROM Products WHERE (Id=?)");
    //this will ask for product common parameters
    query1.addBindValue(id);
    query1.exec();
    query1.first();
    strncpy(subsData->CAS,query1.value(query1.record().indexOf("CAS")).toString().toStdString().c_str(),22);
    if(query1.value(query1.record().indexOf("Family")).toString()=="Alkane") subsData->baseProp.type=FF_Alkane;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Alkene") subsData->baseProp.type=FF_Alkene;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Cycloalkane") subsData->baseProp.type=FF_Cycloalkane;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Aromatic") subsData->baseProp.type=FF_Aromatic;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Water") subsData->baseProp.type=FF_Water;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Alcohol") subsData->baseProp.type=FF_Alcohol;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Polyol") subsData->baseProp.type=FF_Polyol;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Ether") subsData->baseProp.type=FF_Ether;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Ketone") subsData->baseProp.type=FF_Ketone;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Acid") subsData->baseProp.type=FF_Acid;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Ester") subsData->baseProp.type=FF_Ester;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Amine") subsData->baseProp.type=FF_Amine;
    else if(query1.value(query1.record().indexOf("Family")).toString()=="Polymer") subsData->baseProp.type=FF_Polymer;
    else subsData->baseProp.type=0;
    subsData->baseProp.MW=query1.value(query1.record().indexOf("MW")).toDouble();
    subsData->baseProp.MWmono=query1.value(query1.record().indexOf("MonomerMW")).toDouble();
    subsData->baseProp.numMono=1;
    subsData->baseProp.Tc=query1.value(query1.record().indexOf("Tc")).toDouble();
    subsData->baseProp.Pc=query1.value(query1.record().indexOf("Pc")).toDouble();
    subsData->baseProp.Vc=query1.value(query1.record().indexOf("Vc")).toDouble();
    subsData->baseProp.Zc=query1.value(query1.record().indexOf("Zc")).toDouble();
    subsData->baseProp.w=query1.value(query1.record().indexOf("w")).toDouble();
    subsData->baseProp.Zra=query1.value(query1.record().indexOf("Zra")).toDouble();
    subsData->baseProp.r=query1.value(query1.record().indexOf("UniquacR")).toDouble();
    subsData->baseProp.q=query1.value(query1.record().indexOf("UniquacQ")).toDouble();
    subsData->baseProp.qRes=query1.value(query1.record().indexOf("UniquacQres")).toDouble();
    subsData->baseProp.VdWV=query1.value(query1.record().indexOf("VdWV")).toDouble();
    subsData->baseProp.Hf0g=query1.value(query1.record().indexOf("Hf0g")).toDouble();
    subsData->baseProp.Gf0g=query1.value(query1.record().indexOf("Gf0g")).toDouble();
    subsData->baseProp.S0g=query1.value(query1.record().indexOf("S0g")).toDouble();
    subsData->baseProp.Pa=query1.value(query1.record().indexOf("Pa")).toDouble();
    subsData->baseProp.Vliq=query1.value(query1.record().indexOf("Vliq")).toDouble();
    if ((subsData->baseProp.Vliq>0)&&(subsData->baseProp.VdWV>0)) subsData->baseProp.FV=subsData->baseProp.Vliq-1.2*subsData->baseProp.VdWV;
    else subsData->baseProp.FV=0;//Calculation of the free volume
    subsData->baseProp.mu=query1.value(query1.record().indexOf("mu")).toDouble();
    subsData->baseProp.Q=query1.value(query1.record().indexOf("Q")).toDouble();
    subsData->baseProp.RadGyr=query1.value(query1.record().indexOf("RadGyr")).toDouble();
    subsData->baseProp.Tm=query1.value(query1.record().indexOf("Tm")).toDouble();
    subsData->baseProp.Hm=query1.value(query1.record().indexOf("Hm")).toDouble();
    subsData->baseProp.Tb=query1.value(query1.record().indexOf("Tb")).toDouble();
    subsData->baseProp.Hildebrand=query1.value(query1.record().indexOf("Hildebrand")).toDouble();
    subsData->baseProp.HansenD=query1.value(query1.record().indexOf("HansenD")).toDouble();
    subsData->baseProp.HansenP=query1.value(query1.record().indexOf("HansenP")).toDouble();
    subsData->baseProp.HansenH=query1.value(query1.record().indexOf("HansenH")).toDouble();
    subsData->RI.x=query1.value(query1.record().indexOf("RITemp")).toDouble();
    subsData->RI.y=query1.value(query1.record().indexOf("RI")).toDouble();
    subsData->cp0.x=query1.value(query1.record().indexOf("Cp0Temp")).toDouble();
    subsData->cp0.y=query1.value(query1.record().indexOf("Cp0")).toDouble();
    subsData->vp.x=query1.value(query1.record().indexOf("VpTemp")).toDouble();
    subsData->vp.y=query1.value(query1.record().indexOf("Vp")).toDouble();
    subsData->hVsat.x=query1.value(query1.record().indexOf("HvTemp")).toDouble();
    subsData->hVsat.y=query1.value(query1.record().indexOf("Hv")).toDouble();
    subsData->lCp.x=query1.value(query1.record().indexOf("CplTemp")).toDouble();
    subsData->lCp.y=query1.value(query1.record().indexOf("Cpl")).toDouble();
    subsData->lDens.x=query1.value(query1.record().indexOf("LiqDensTemp")).toDouble();
    subsData->lDens.y=query1.value(query1.record().indexOf("LiqDens")).toDouble();
    subsData->lVisc.x=query1.value(query1.record().indexOf("LiqViscTemp")).toDouble();
    subsData->lVisc.y=query1.value(query1.record().indexOf("LiqVisc")).toDouble();
    subsData->lThC.x=query1.value(query1.record().indexOf("LiqThCondTemp")).toDouble();
    subsData->lThC.y=query1.value(query1.record().indexOf("LiqThCond")).toDouble();
    subsData->lIsothComp.x=query1.value(query1.record().indexOf("LiqIsothCompTemp")).toDouble();
    subsData->lIsothComp.y=query1.value(query1.record().indexOf("LiqIsothComp")).toDouble();
    //printf("MW:%f\n",query1.value(query1.record().indexOf("MW")).toDouble());

    for(i=0;i<10;i++){
        subsData->UnifStdSubg[i][0]=subsData->UnifStdSubg[i][1]=0;
        subsData->UnifPSRKSubg[i][0]=subsData->UnifPSRKSubg[i][1]=0;
        subsData->UnifDortSubg[i][0]=subsData->UnifDortSubg[i][1]=0;
        subsData->UnifNistSubg[i][0]=subsData->UnifNistSubg[i][1]=0;
    }
    query1.prepare("SELECT * FROM Products_UnifacSt WHERE (IdProduct=?)");
    query1.addBindValue(id);
    query1.exec();
    i=0;
    while (query1.next()) {
        subsData->UnifStdSubg[i][0]=query1.value(query1.record().indexOf("UnifacSubgroup")).toInt();
        subsData->UnifStdSubg[i][1]=query1.value(query1.record().indexOf("Number")).toInt();
        i++;
    }
    query1.prepare("SELECT * FROM Products_UnifacPSRK WHERE (IdProduct=?)");
    query1.addBindValue(id);
    query1.exec();
    i=0;
    while (query1.next()) {
        subsData->UnifPSRKSubg[i][0]=query1.value(query1.record().indexOf("UnifacSubgroup")).toInt();
        subsData->UnifPSRKSubg[i][1]=query1.value(query1.record().indexOf("Number")).toInt();
        i++;
    }
    query1.prepare("SELECT * FROM Products_UnifacDort WHERE (IdProduct=?)");
    query1.addBindValue(id);
    query1.exec();
    i=0;
    while (query1.next()) {
        subsData->UnifDortSubg[i][0]=query1.value(query1.record().indexOf("UnifacSubgroup")).toInt();
        subsData->UnifDortSubg[i][1]=query1.value(query1.record().indexOf("Number")).toInt();
        i++;
    }
    query1.prepare("SELECT * FROM Products_UnifacNist WHERE (IdProduct=?)");
    query1.addBindValue(id);
    query1.exec();
    i=0;
    while (query1.next()) {
        subsData->UnifNistSubg[i][0]=query1.value(query1.record().indexOf("UnifacSubgroup")).toInt();
        subsData->UnifNistSubg[i][1]=query1.value(query1.record().indexOf("Number")).toInt();
        i++;
    }
}


void GetEosData(const int *IdProduct,enum FF_EOS *eos,const int *IdEos,const int *IdCorrParam,QSqlDatabase *db,void *dataV, FF_Correlation *cp0)
{
    QSqlQuery query1(*db),query2(*db),query4(*db);
    query1.prepare("SELECT MW,Tc,Pc,w,Zc,VdWV,Cp0Form,Cp0A,Cp0B,Cp0C,Cp0D,Cp0E,Cp0LimI,Cp0LimS FROM Products WHERE (Id=?)");//this will ask for product common parameters
    query1.addBindValue(*IdProduct);
    query1.exec();
    query1.first();
    //printf("Buenas bifurcamos acceso a BBDD por EOS\n");
    //printf("MW:%f\n",query1.value(query1.record().indexOf("MW")).toDouble());
    switch (*eos)//Depending on the eos we will query different tables
    {
        case FF_PCSAFT:
        case FF_PPCSAFT_GV:
        case FF_PPCSAFT_JC:
        case FF_PCSAFTPOL1:
        {
            //printf("Hola soy FF_PCSAFT en BD\n");
            //*( FF_SaftEOSdata*) data;
             FF_SaftEOSdata *data=( FF_SaftEOSdata*)dataV;//We dereference dataV as a FF_SaftEOSdata pointer and asign its address value to data.
            //So data and dataV contain the same address value
            query2.prepare("SELECT MW,Tc,Pc,Zc,sigma,m,epsilon,kAB,epsilonAB,mu,xp,nPos,nNeg,nAcid FROM EosParam WHERE (Id=?)");//and this for eos parameters
            query2.addBindValue(*IdEos);
            query2.exec();
            query2.first();
            if (query2.value(query2.record().indexOf("MW")).toDouble() > 0) data->MW=query2.value(query2.record().indexOf("MW")).toDouble();
            else data->MW=query1.value(query1.record().indexOf("MW")).toDouble();
            if (query2.value(query2.record().indexOf("Tc")).toDouble() > 0) data->Tc=query2.value(query2.record().indexOf("Tc")).toDouble();
            else data->Tc=query1.value(query1.record().indexOf("Tc")).toDouble();
            if (query2.value(query2.record().indexOf("Pc")).toDouble() > 0) data->Pc=query2.value(query2.record().indexOf("Pc")).toDouble();
            else data->Pc=query1.value(query1.record().indexOf("Pc")).toDouble();
            if (query2.value(query2.record().indexOf("Zc")).toDouble() > 0) data->Zc=query2.value(query2.record().indexOf("Zc")).toDouble();
            else data->Zc=query1.value(query1.record().indexOf("Zc")).toDouble();
            data->w=query1.value(query1.record().indexOf("w")).toDouble();
            data->sigma=query2.value(query2.record().indexOf("sigma")).toDouble();
            data->m=query2.value(query2.record().indexOf("m")).toDouble();
            if (*eos==FF_PCSAFTPOL1){
                data->m=data->MW*data->m;
                //*eos=FF_PCSAFT;
            }
            data->epsilon=query2.value(query2.record().indexOf("epsilon")).toDouble();
            data->kAB=query2.value(query2.record().indexOf("kAB")).toDouble();
            data->epsilonAB=query2.value(query2.record().indexOf("epsilonAB")).toDouble();
            data->mu=query2.value(query2.record().indexOf("mu")).toDouble();
            data->xp=query2.value(query2.record().indexOf("xp")).toDouble();
            data->nPos=query2.value(query2.record().indexOf("nPos")).toInt();
            data->nNeg=query2.value(query2.record().indexOf("nNeg")).toInt();
            data->nAcid=query2.value(query2.record().indexOf("nAcid")).toInt();
            //printf("MW:%f Tc:%f Pc:%f Zc:%f sigma:%f m:%f epsilon:%f kAB:%f epsilonAB:%f nPos:%i nNeg:%i nAcid:%i\n",
            //       data->MW,data->Tc,data->Pc,data->Zc,data->sigma,data->m,data->epsilon,data->kAB,data->epsilonAB,data->nPos,data->nNeg,data->nAcid);
        }
            break;
        case FF_SW:
        case FF_IAPWS95:
        {
         FF_SWEOSdata *data=( FF_SWEOSdata*)dataV;
        query2.prepare("SELECT MW,Tc,Pc,Zc,tRef,rhoRef,nPol,nExp,nSpec,nFinal,Tmin,Tmax,Pmax FROM EosParam WHERE (Id=?)");//and this for eos parameters
        query2.addBindValue(*IdEos);
        query2.exec();
        query2.first();
        if (query2.value(query2.record().indexOf("MW")).toDouble() > 0) data->MW=query2.value(query2.record().indexOf("MW")).toDouble();
        else data->MW=query1.value(query1.record().indexOf("MW")).toDouble();
        if (query2.value(query2.record().indexOf("Tc")).toDouble() > 0) data->Tc=query2.value(query2.record().indexOf("Tc")).toDouble();
        else data->Tc=query1.value(query1.record().indexOf("Tc")).toDouble();
        if (query2.value(query2.record().indexOf("Pc")).toDouble() > 0) data->Pc=query2.value(query2.record().indexOf("Pc")).toDouble();
        else data->Pc=query1.value(query1.record().indexOf("Pc")).toDouble();
        if (query2.value(query2.record().indexOf("Zc")).toDouble() > 0) data->Zc=query2.value(query2.record().indexOf("Zc")).toDouble();
        else data->Zc=query1.value(query1.record().indexOf("Zc")).toDouble();
        data->w=query1.value(query1.record().indexOf("w")).toDouble();
        data->tRef=query2.value(query2.record().indexOf("tRef")).toDouble();
        data->rhoRef=query2.value(query2.record().indexOf("rhoRef")).toDouble();
        data->nPol=query2.value(query2.record().indexOf("nPol")).toInt();
        data->nExp=query2.value(query2.record().indexOf("nExp")).toInt();
        data->nSpec=query2.value(query2.record().indexOf("nSpec")).toInt();
        data->nFinal=query2.value(query2.record().indexOf("nFinal")).toInt();

        //printf("MW:%f Tc:%f Pc:%f Zc:%f tRef:%f rhoRef:%f nPol:%i nExp:%i nSpec:%i\n",data->MW,data->Tc,data->Pc,data->Zc,data->tRef,data->rhoRef,data->nPol,data->nExp,data->nSpec);
        //printf("N.eos:%i\n",*IdEos);
        QSqlQuery query3(*db);
        query3.prepare("SELECT * FROM SWparam WHERE (IdEos=?) ORDER BY Position");
        query3.addBindValue(*IdEos);
        query3.exec();
        query3.first();
        int i,j;
        do
        {
            i=query3.value(query3.record().indexOf("Position")).toInt();
            //printf("position:%i\n",i);
            data->n[i-1]=query3.value(query3.record().indexOf("n")).toDouble();
            data->d[i-1]=query3.value(query3.record().indexOf("d")).toInt();
            data->t[i-1]=query3.value(query3.record().indexOf("t")).toDouble();
            if ((i > data->nPol) && (i<=(data->nPol+data->nExp))) data->c[i-1]=query3.value(query3.record().indexOf("c")).toInt();
            //printf("n:%f d:%i t:%f c:%i\n",data->n[i-1],data->d[i-1],data->t[i-1],data->c[i-1]);
            if ((i > (data->nPol+data->nExp)) && (i<=(data->nPol+data->nExp+data->nSpec)))
            {
               //printf("%i\n",i);
               j=i-data->nPol-data->nExp-1;
               data->a[j]=query3.value(query3.record().indexOf("alpha")).toDouble();
               data->e[j]=query3.value(query3.record().indexOf("epsilon")).toDouble();
               data->b[j]=query3.value(query3.record().indexOf("beta")).toDouble();
               data->g[j]=query3.value(query3.record().indexOf("gamma")).toDouble();
            }
            if ((i > (data->nPol+data->nExp+data->nSpec)) && (i<=(data->nPol+data->nExp+data->nSpec+data->nFinal)))
            {
               //printf("%i\n",i);
               j=i-data->nPol-data->nExp-data->nSpec-1;
               data->af[j]=query3.value(query3.record().indexOf("a")).toDouble();
               data->bf[j]=query3.value(query3.record().indexOf("b")).toDouble();
               data->Af[j]=query3.value(query3.record().indexOf("Af")).toDouble();
               data->Bf[j]=query3.value(query3.record().indexOf("Bf")).toDouble();
               data->Cf[j]=query3.value(query3.record().indexOf("Cf")).toDouble();
               data->Df[j]=query3.value(query3.record().indexOf("Df")).toDouble();
               data->betaf[j]=query3.value(query3.record().indexOf("betaf")).toDouble();
            }


        } while (query3.next());
        }
            break;
        default:
        {
            //*( FF_CubicEOSdata*) data;
             FF_CubicEOSdata *data=( FF_CubicEOSdata*)dataV;
            query2.prepare("SELECT MW,Tc,Pc,Zc,w,c,k1,k2,k3,k4 FROM EosParam WHERE (Id=?)");//and this for eos parameters
            query2.addBindValue(*IdEos);
            query2.exec();
            query2.first();
            //k1Num=query2.record().indexOf("k1");
            //k2Num=query2.record().indexOf("k2");
            //k3Num=query2.record().indexOf("k3");
            if (query2.value(query2.record().indexOf("MW")).toDouble() > 0) data->MW=query2.value(query2.record().indexOf("MW")).toDouble();
            else data->MW=query1.value(query1.record().indexOf("MW")).toDouble();
            if (query2.value(query2.record().indexOf("Tc")).toDouble() > 0) data->Tc=query2.value(query2.record().indexOf("Tc")).toDouble();
            else data->Tc=query1.value(query1.record().indexOf("Tc")).toDouble();
            if (query2.value(query2.record().indexOf("Pc")).toDouble() > 0) data->Pc=query2.value(query2.record().indexOf("Pc")).toDouble();
            else data->Pc=query1.value(query1.record().indexOf("Pc")).toDouble();
            if (query2.value(query2.record().indexOf("Zc")).toDouble() > 0.0) data->Zc=query2.value(query2.record().indexOf("Zc")).toDouble();
            else data->Zc=query1.value(query1.record().indexOf("Zc")).toDouble();
            if (query2.value(query2.record().indexOf("w")).toDouble() > 0.0) data->w=query2.value(query2.record().indexOf("w")).toDouble();
            else data->w=query1.value(query1.record().indexOf("w")).toDouble();
            data->VdWV=query1.value(query1.record().indexOf("VdWV")).toDouble();
            data->c=query2.value(query2.record().indexOf("c")).toDouble();
            data->k1=query2.value(query2.record().indexOf("k1")).toDouble();
            data->k2=query2.value(query2.record().indexOf("k2")).toDouble();
            data->k3=query2.value(query2.record().indexOf("k3")).toDouble();
            data->k4=query2.value(query2.record().indexOf("k4")).toDouble();
            //printf("MW:%f Tc:%f Pc:%f w:%f VdWV:%f k1:%f k2:%f k3:%f k4:%f\n",data->MW,data->Tc,data->Pc,data->w,data->VdWV,data->k1,data->k2,data->k3,data->k4);
        }
            break;
    }

    query4.prepare("SELECT * FROM CorrelationParam WHERE (Id=?)");//this will ask for Cp0 correlation parameters
    query4.addBindValue(*IdCorrParam);
    query4.exec();
    query4.first();
    cp0->form =query4.value(query4.record().indexOf("NumCorrelation")).toInt();
    cp0->coef[0] =query4.value(query4.record().indexOf("A")).toDouble();
    cp0->coef[1] =query4.value(query4.record().indexOf("B")).toDouble();
    cp0->coef[2] =query4.value(query4.record().indexOf("C")).toDouble();
    cp0->coef[3] =query4.value(query4.record().indexOf("D")).toDouble();
    cp0->coef[4] =query4.value(query4.record().indexOf("E")).toDouble();
    cp0->coef[5] =query4.value(query4.record().indexOf("F")).toDouble();
    cp0->coef[6] =query4.value(query4.record().indexOf("G")).toDouble();
    cp0->coef[7] =query4.value(query4.record().indexOf("H")).toDouble();
    cp0->coef[8] =query4.value(query4.record().indexOf("I")).toDouble();
    cp0->coef[9] =query4.value(query4.record().indexOf("J")).toDouble();
    cp0->coef[10] =query4.value(query4.record().indexOf("K")).toDouble();
    cp0->coef[11] =query4.value(query4.record().indexOf("L")).toDouble();
    cp0->coef[12] =query4.value(query4.record().indexOf("M")).toDouble();
    //cp0->limI =query4.value(query4.record().indexOf("Tmin")).toDouble();
    //cp0->limS =query4.value(query4.record().indexOf("Tmax")).toDouble();
    //printf("form:%i A:%f B:%f C:%f D:%f E:%f\n",cp0->form,cp0->A,cp0->B,cp0->C,cp0->D,cp0->E);
}

void GetEOSData(int *eosType,FF_SubstanceData *subsData,QSqlDatabase *db)
{
    QSqlQuery query1(*db),query2(*db);
    std::string eosString;//To store the eos model in string format
    //We query the Products table for general data
    query1.prepare("SELECT MW,Tc,Pc,w,Zc,VdWV FROM Products WHERE (Id=?)");//this will ask for product common parameters
    query1.addBindValue(subsData->id);
    query1.exec();
    query1.first();
    //printf("MW:%f\n",query1.value(query1.record().indexOf("MW")).toDouble());
    if (*eosType==FF_SAFTtype)//Depending on the eos we query different fields from the EoPparam table, and store the result in different places of the Substance object
    {
            //printf("Hola soy FF_PCSAFT en BD\n");
            query2.prepare("SELECT Eos,MW,Tc,Pc,Zc,sigma,m,epsilon,lambdaA,lambdaR,chi,kAB,epsilonAB,mu,xp,nPos,nNeg,nAcid FROM EosParam WHERE (Id=?)");//and this for eos parameters
            query2.addBindValue(subsData->saftData.id);
            query2.exec();
            query2.first();
            eosString=query2.value(query2.record().indexOf("Eos")).toString().toStdString();
            ConvertEosToEnumeration2(&eosString,&subsData->saftData.eos);
            if (query2.value(query2.record().indexOf("MW")).toDouble() > 0) subsData->saftData.MW=query2.value(query2.record().indexOf("MW")).toDouble();
            else subsData->saftData.MW=query1.value(query1.record().indexOf("MW")).toDouble();
            if (query2.value(query2.record().indexOf("Tc")).toDouble() > 0) subsData->saftData.Tc=query2.value(query2.record().indexOf("Tc")).toDouble();
            else subsData->saftData.Tc=query1.value(query1.record().indexOf("Tc")).toDouble();
            if (query2.value(query2.record().indexOf("Pc")).toDouble() > 0) subsData->saftData.Pc=query2.value(query2.record().indexOf("Pc")).toDouble();
            else subsData->saftData.Pc=query1.value(query1.record().indexOf("Pc")).toDouble();
            if (query2.value(query2.record().indexOf("Zc")).toDouble() > 0) subsData->saftData.Zc=query2.value(query2.record().indexOf("Zc")).toDouble();
            else subsData->saftData.Zc=query1.value(query1.record().indexOf("Zc")).toDouble();
            subsData->saftData.w=query1.value(query1.record().indexOf("w")).toDouble();
            subsData->saftData.sigma=query2.value(query2.record().indexOf("sigma")).toDouble();
            subsData->saftData.m=query2.value(query2.record().indexOf("m")).toDouble();
            if (query2.value(query2.record().indexOf("Eos")).toString()=="PCSAFTPOL1"){
                subsData->saftData.m=subsData->saftData.MW*subsData->saftData.m;//for polymers
            }
            subsData->saftData.epsilon=query2.value(query2.record().indexOf("epsilon")).toDouble();
            subsData->saftData.la=query2.value(query2.record().indexOf("lambdaA")).toDouble();
            subsData->saftData.lr=query2.value(query2.record().indexOf("lambdaR")).toDouble();
            subsData->saftData.chi=query2.value(query2.record().indexOf("chi")).toDouble();
            subsData->saftData.kAB=query2.value(query2.record().indexOf("kAB")).toDouble();
            subsData->saftData.epsilonAB=query2.value(query2.record().indexOf("epsilonAB")).toDouble();
            subsData->saftData.mu=query2.value(query2.record().indexOf("mu")).toDouble();
            subsData->saftData.xp=query2.value(query2.record().indexOf("xp")).toDouble();
            subsData->saftData.nPos=query2.value(query2.record().indexOf("nPos")).toInt();
            subsData->saftData.nNeg=query2.value(query2.record().indexOf("nNeg")).toInt();
            subsData->saftData.nAcid=query2.value(query2.record().indexOf("nAcid")).toInt();
            //printf("MW:%f Tc:%f Pc:%f Zc:%f sigma:%f m:%f epsilon:%f kAB:%f epsilonAB:%f nPos:%i nNeg:%i nAcid:%i\n",
            //       subsData->saftData.MW,subsData->saftData.Tc,subsData->saftData.Pc,subsData->saftData.Zc,subsData->saftData.sigma,subsData->saftData.m,
            //subsData->saftData.epsilon,subsData->saftData.kAB,subsData->saftData.epsilonAB,subsData->saftData.nPos,subsData->saftData.nNeg,subsData->saftData.nAcid);
    }
    else if (*eosType==FF_SWtype)//Span and Wagner type eos
    {
        query2.prepare("SELECT Eos,MW,Tc,Pc,Zc,tRef,rhoRef,nPol,nExp,nSpec,nFinal,Tmin,Tmax,Pmax FROM EosParam WHERE (Id=?)");//and this for eos parameters
        query2.addBindValue(subsData->swData.id);
        query2.exec();
        query2.first();
        eosString=query2.value(query2.record().indexOf("Eos")).toString().toStdString();
        ConvertEosToEnumeration2(&eosString,&subsData->swData.eos);
        if (query2.value(query2.record().indexOf("MW")).toDouble() > 0) subsData->swData.MW=query2.value(query2.record().indexOf("MW")).toDouble();
        else subsData->swData.MW=query1.value(query1.record().indexOf("MW")).toDouble();
        if (query2.value(query2.record().indexOf("Tc")).toDouble() > 0) subsData->swData.Tc=query2.value(query2.record().indexOf("Tc")).toDouble();
        else subsData->swData.Tc=query1.value(query1.record().indexOf("Tc")).toDouble();
        if (query2.value(query2.record().indexOf("Pc")).toDouble() > 0) subsData->swData.Pc=query2.value(query2.record().indexOf("Pc")).toDouble();
        else subsData->swData.Pc=query1.value(query1.record().indexOf("Pc")).toDouble();
        if (query2.value(query2.record().indexOf("Zc")).toDouble() > 0) subsData->swData.Zc=query2.value(query2.record().indexOf("Zc")).toDouble();
        else subsData->swData.Zc=query1.value(query1.record().indexOf("Zc")).toDouble();
        subsData->swData.w=query1.value(query1.record().indexOf("w")).toDouble();
        subsData->swData.tRef=query2.value(query2.record().indexOf("tRef")).toDouble();
        subsData->swData.rhoRef=query2.value(query2.record().indexOf("rhoRef")).toDouble();
        subsData->swData.nPol=query2.value(query2.record().indexOf("nPol")).toInt();
        subsData->swData.nExp=query2.value(query2.record().indexOf("nExp")).toInt();
        subsData->swData.nSpec=query2.value(query2.record().indexOf("nSpec")).toInt();
        subsData->swData.nFinal=query2.value(query2.record().indexOf("nFinal")).toInt();

        //printf("MW:%f Tc:%f Pc:%f Zc:%f tRef:%f rhoRef:%f nPol:%i nExp:%i nSpec:%i nFinal:%i\n",subsData->swData.MW,subsData->swData.Tc,subsData->swData.Pc,subsData->swData.Zc,
        //subsData->swData.tRef,subsData->swData.rhoRef,subsData->swData.nPol,subsData->swData.nExp,subsData->swData.nSpec,subsData->swData.nFinal);
        //printf("N.eos:%i\n",subsData->swData.id);
        QSqlQuery query3(*db);
        query3.prepare("SELECT * FROM SWparam WHERE (IdEos=?) ORDER BY Position");
        query3.addBindValue(subsData->swData.id);
        query3.exec();
        query3.first();
        int i,j;
        do
        {
            i=query3.value(query3.record().indexOf("Position")).toInt();
            //printf("position:%i\n",i);
            subsData->swData.n[i-1]=query3.value(query3.record().indexOf("n")).toDouble();
            subsData->swData.d[i-1]=query3.value(query3.record().indexOf("d")).toInt();
            subsData->swData.t[i-1]=query3.value(query3.record().indexOf("t")).toDouble();
            if ((i > subsData->swData.nPol) && (i<=(subsData->swData.nPol+subsData->swData.nExp))) subsData->swData.c[i-1]=query3.value(query3.record().indexOf("c")).toInt();
            //printf("n:%f d:%i t:%f c:%i\n",subsData->swData.n[i-1],subsData->swData.d[i-1],subsData->swData.t[i-1],subsData->swData.c[i-1]);
            if ((i > (subsData->swData.nPol+subsData->swData.nExp)) && (i<=(subsData->swData.nPol+subsData->swData.nExp+subsData->swData.nSpec)))
            {
               j=i-subsData->swData.nPol-subsData->swData.nExp-1;
               subsData->swData.a[j]=query3.value(query3.record().indexOf("alpha")).toDouble();
               subsData->swData.e[j]=query3.value(query3.record().indexOf("epsilon")).toDouble();
               subsData->swData.b[j]=query3.value(query3.record().indexOf("beta")).toDouble();
               subsData->swData.g[j]=query3.value(query3.record().indexOf("gamma")).toDouble();
               //printf("%i a:%f e:%f b:%f g:%f\n",j+1,subsData->swData.a[j],subsData->swData.e[j],subsData->swData.b[j],subsData->swData.g[j]);
            }
            if ((i > (subsData->swData.nPol+subsData->swData.nExp+subsData->swData.nSpec)) && (i<=(subsData->swData.nPol+subsData->swData.nExp+subsData->swData.nSpec+subsData->swData.nFinal)))
            {
               j=i-subsData->swData.nPol-subsData->swData.nExp-subsData->swData.nSpec-1;
               subsData->swData.af[j]=query3.value(query3.record().indexOf("a")).toDouble();
               subsData->swData.bf[j]=query3.value(query3.record().indexOf("b")).toDouble();
               subsData->swData.Af[j]=query3.value(query3.record().indexOf("Af")).toDouble();
               subsData->swData.Bf[j]=query3.value(query3.record().indexOf("Bf")).toDouble();
               subsData->swData.Cf[j]=query3.value(query3.record().indexOf("Cf")).toDouble();
               subsData->swData.Df[j]=query3.value(query3.record().indexOf("Df")).toDouble();
               subsData->swData.betaf[j]=query3.value(query3.record().indexOf("betaf")).toDouble();
               //printf("%i af:%f bf:%f Af:%f Bf:%f Cf:%f Df:%f betaf:%f\n",j+1,subsData->swData.af[j],subsData->swData.bf[j],subsData->swData.Af[j],subsData->swData.Bf[j],subsData->swData.Cf[j],
               //       subsData->swData.Df[j],subsData->swData.betaf[j]);
            }


        } while (query3.next());
    }
    else if ((*eosType==FF_CubicType)||(*eosType==FF_CubicPRtype)||(*eosType==FF_CubicSRKtype))//
        {
            query2.prepare("SELECT Eos,MW,Tc,Pc,Zc,w,c,k1,k2,k3,k4 FROM EosParam WHERE (Id=?)");//and this for eos parameters
            query2.addBindValue(subsData->cubicData.id);
            query2.exec();
            query2.first();
            eosString=query2.value(query2.record().indexOf("Eos")).toString().toStdString();
            ConvertEosToEnumeration2(&eosString,&subsData->cubicData.eos);
            //k1Num=query2.record().indexOf("k1");
            //k2Num=query2.record().indexOf("k2");
            //k3Num=query2.record().indexOf("k3");
            if (query2.value(query2.record().indexOf("MW")).toDouble() > 0) subsData->cubicData.MW=query2.value(query2.record().indexOf("MW")).toDouble();
            else subsData->cubicData.MW=query1.value(query1.record().indexOf("MW")).toDouble();
            if (query2.value(query2.record().indexOf("Tc")).toDouble() > 0) subsData->cubicData.Tc=query2.value(query2.record().indexOf("Tc")).toDouble();
            else subsData->cubicData.Tc=query1.value(query1.record().indexOf("Tc")).toDouble();
            if (query2.value(query2.record().indexOf("Pc")).toDouble() > 0) subsData->cubicData.Pc=query2.value(query2.record().indexOf("Pc")).toDouble();
            else subsData->cubicData.Pc=query1.value(query1.record().indexOf("Pc")).toDouble();
            if (query2.value(query2.record().indexOf("Zc")).toDouble() > 0) subsData->cubicData.Zc=query2.value(query2.record().indexOf("Zc")).toDouble();
            else subsData->cubicData.Zc=query1.value(query1.record().indexOf("Zc")).toDouble();
            if (query2.value(query2.record().indexOf("w")).toDouble() > 0) subsData->cubicData.w=query2.value(query2.record().indexOf("w")).toDouble();
            else subsData->cubicData.w=query1.value(query1.record().indexOf("w")).toDouble();
            subsData->cubicData.VdWV=query1.value(query1.record().indexOf("VdWV")).toDouble();
            subsData->cubicData.c=query2.value(query2.record().indexOf("c")).toDouble();
            subsData->cubicData.k1=query2.value(query2.record().indexOf("k1")).toDouble();
            subsData->cubicData.k2=query2.value(query2.record().indexOf("k2")).toDouble();
            subsData->cubicData.k3=query2.value(query2.record().indexOf("k3")).toDouble();
            subsData->cubicData.k4=query2.value(query2.record().indexOf("k4")).toDouble();
            //printf("MW:%f Tc:%f Pc:%f w:%f VdWV:%f k1:%f k2:%f k3:%f k4:%f\n",subsData->cubicData.MW,subsData->cubicData.Tc,subsData->cubicData.Pc,
            //subsData->cubicData.w,subsData->cubicData.VdWV,subsData->cubicData.k1,subsData->cubicData.k2,subsData->cubicData.k3,subs->cubicData.k4);
        }
}



void GetCorrDataByType(const int *IdProduct,const QString *type,QSqlDatabase *db,int *corrNum,double coef[])//
{
    QSqlQuery query1(*db);

    query1.prepare("SELECT * FROM CorrelationParam INNER JOIN (Correlations INNER JOIN "
                   "PhysProp ON Correlations.IdPhysProp = PhysProp.Id) "
                   "ON CorrelationParam.NumCorrelation = Correlations.Number WHERE "
                   "(((CorrelationParam.IdProduct)=?) AND ((PhysProp.Property)=?));");


    query1.addBindValue(*IdProduct);
    query1.addBindValue(*type);
    query1.exec();
    query1.first();
    *corrNum=query1.value(query1.record().indexOf("NumCorrelation")).toInt();
    coef[0]=query1.value(query1.record().indexOf("A")).toDouble();
    coef[1]=query1.value(query1.record().indexOf("B")).toDouble();
    coef[2]=query1.value(query1.record().indexOf("C")).toDouble();
    coef[3]=query1.value(query1.record().indexOf("D")).toDouble();
    coef[4]=query1.value(query1.record().indexOf("E")).toDouble();
    coef[5]=query1.value(query1.record().indexOf("F")).toDouble();
    //printf("SubsId:%i A:%f\n",*IdProduct,query1.value(query1.record().indexOf("A")).toDouble());
    //std::cout<<query1.value(query1.record().indexOf("Type")).toString().toStdString()<<std::endl;
}


void GetCorrDataById( FF_Correlation *corr,QSqlDatabase *db)//
{
    QSqlQuery query1(*db);
    query1.prepare("SELECT * FROM Products INNER JOIN CorrelationParam "
                   "ON Products.Id = CorrelationParam.IdProduct WHERE (((CorrelationParam.Id)=?))");
    //query1.prepare("SELECT * FROM Products INNER JOIN CorrelationParam ON Products.Id = CorrelationParam.IdProduct WHERE"
    //               " (CorrelationParam.Id=?)");
    query1.addBindValue(corr->id);
    query1.exec();
    query1.first();
    //*MW=query1.value(query1.record().indexOf("MW")).toDouble();
    corr->form=query1.value(query1.record().indexOf("NumCorrelation")).toInt();
    corr->coef[0]=query1.value(query1.record().indexOf("A")).toDouble();
    corr->coef[1]=query1.value(query1.record().indexOf("B")).toDouble();
    corr->coef[2]=query1.value(query1.record().indexOf("C")).toDouble();
    corr->coef[3]=query1.value(query1.record().indexOf("D")).toDouble();
    corr->coef[4]=query1.value(query1.record().indexOf("E")).toDouble();
    corr->coef[5]=query1.value(query1.record().indexOf("F")).toDouble();
    corr->coef[6]=query1.value(query1.record().indexOf("G")).toDouble();
    corr->coef[7]=query1.value(query1.record().indexOf("H")).toDouble();
    corr->coef[8]=query1.value(query1.record().indexOf("I")).toDouble();
    corr->coef[9]=query1.value(query1.record().indexOf("J")).toDouble();
    corr->coef[10]=query1.value(query1.record().indexOf("K")).toDouble();
    corr->coef[11]=query1.value(query1.record().indexOf("L")).toDouble();
    corr->coef[12]=query1.value(query1.record().indexOf("M")).toDouble();
    corr->coef[13]=query1.value(query1.record().indexOf("N")).toDouble();
    corr->limI=query1.value(query1.record().indexOf("Tmin")).toDouble();
    corr->limS=query1.value(query1.record().indexOf("Tmax")).toDouble();
    //printf("corr.form:%i\n",corr->form);
    //printf("%f %f %f %f %f %f %f %f %f %f \n",corr->coef[0],corr->coef[1],corr->coef[2],corr->coef[3],corr->coef[4],corr->coef[5],
    //corr->coef[6],corr->coef[7],corr->coef[8],corr->coef[9]);
    //printf("Tmin, Tmax: %f %f\n",corr->limI,corr->limS);
}

//Adds a new eos to the database
void AddEosToDataBase(int idSubs,enum FF_EosType eosType,void *eosData,double *Tmin,double *Tmax, QString *description,QSqlDatabase *db){
    QSqlQuery query(*db);
    QString eos;
    if (eosType==FF_CubicType){
        FF_CubicEOSdata *cubicData=(FF_CubicEOSdata*)eosData;
        ConvertEnumerationToEos(&cubicData->eos,&eos);
        query.prepare("INSERT INTO EosParam(IdProduct,Eos,MW,Tc,Pc,Zc,w,c,k1,k2,k3,k4,Tmin,Tmax,Description) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
        query.addBindValue(idSubs);
        query.addBindValue(eos);
        query.addBindValue(cubicData->MW);
        query.addBindValue(cubicData->Tc);
        query.addBindValue(cubicData->Pc);
        query.addBindValue(cubicData->Zc);
        query.addBindValue(cubicData->w);
        query.addBindValue(cubicData->c);
        query.addBindValue(cubicData->k1);
        query.addBindValue(cubicData->k2);
        query.addBindValue(cubicData->k3);
        query.addBindValue(cubicData->k4);
        query.addBindValue(*Tmin);
        query.addBindValue(*Tmax);
        query.addBindValue(*description);
        query.exec();
    }
    else if(eosType==FF_SAFTtype){
        FF_SaftEOSdata *saftData=(FF_SaftEOSdata*)eosData;
        ConvertEnumerationToEos(&saftData->eos,&eos);
        //printf("%i %i %f %f %f %f %f %f %f %f\n",idSubs,saftData->eos,saftData->MW,saftData->Tc,saftData->Pc,saftData->Zc,saftData->w,saftData->sigma,saftData->m,saftData->epsilon);
        //printf("%f %f %f %f %f %f %i %i %i\n",saftData->la,saftData->lr,saftData->kAB,saftData->epsilonAB,saftData->mu,saftData->xp,saftData->nPos,saftData->nNeg,saftData->nAcid);
        //printf("%f %f %s\n",*Tmin,*Tmax,*description);
        query.prepare("INSERT INTO EosParam(IdProduct,Eos,MW,Tc,Pc,Zc,w,sigma,m,epsilon,lambdaA,lambdaR,kAB,epsilonAB,mu,xp,nPos,nNeg,nAcid,Tmin,Tmax,Description) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
        query.addBindValue(idSubs);
        query.addBindValue(eos);
        query.addBindValue(saftData->MW);
        query.addBindValue(saftData->Tc);
        query.addBindValue(saftData->Pc);
        query.addBindValue(saftData->Zc);
        query.addBindValue(saftData->w);
        query.addBindValue(saftData->sigma);
        query.addBindValue(saftData->m);
        query.addBindValue(saftData->epsilon);
        query.addBindValue(saftData->la);
        query.addBindValue(saftData->lr);
        query.addBindValue(saftData->kAB);
        query.addBindValue(saftData->epsilonAB);
        query.addBindValue(saftData->mu);
        query.addBindValue(saftData->xp);
        query.addBindValue(saftData->nPos);
        query.addBindValue(saftData->nNeg);
        query.addBindValue(saftData->nAcid);
        query.addBindValue(*Tmin);
        query.addBindValue(*Tmax);
        query.addBindValue(*description);
        query.exec();
        //printf("Inserted\n");
    }

}


//Adds a new correlation to the database
void AddCorrToDataBase(int idSubs,FF_Correlation *corr,double *Tmin,double *Tmax, QString *description,QSqlDatabase *db){
    QSqlQuery query(*db);
        query.prepare("INSERT INTO CorrelationParam(IdProduct,NumCorrelation,A,B,C,D,E,F,Tmin,Tmax,Reference) VALUES (?,?,?,?,?,?,?,?,?,?,?)");
        query.addBindValue(idSubs);
        query.addBindValue(corr->form);
        query.addBindValue(corr->coef[0]);
        query.addBindValue(corr->coef[1]);
        query.addBindValue(corr->coef[2]);
        query.addBindValue(corr->coef[3]);
        query.addBindValue(corr->coef[4]);
        query.addBindValue(corr->coef[5]);
        query.addBindValue(*Tmin);
        query.addBindValue(*Tmax);
        query.addBindValue(*description);
        query.exec();
}

//Writes the Unifac information from the database to a file from where it can be extracted using C
void WriteUnifacToFile(QSqlDatabase *db){
    int sg,g,g1,g2;
    double r,q,A12,B12,C12,A21,B21,C21;
    QSqlQuery query(*db);
    query.prepare("SELECT * FROM UnifacSubgroups");
    query.exec();
    FILE *fStd, *fPSRK, *fDort, *fNist;
    fStd=fopen("UnifacSubgStd.txt","w+");
    fPSRK=fopen("UnifacSubgPSRK.txt","w+");
    fDort=fopen("UnifacSubgDort.txt","w+");
    fNist=fopen("UnifacSubgNist.txt","w+");
    while (query.next()){
        if (query.value(query.record().indexOf("SubgroupVL")).toInt()>0){
            sg=query.value(query.record().indexOf("SubgroupVL")).toInt();
            g=query.value(query.record().indexOf("GroupVL")).toInt();
            r=query.value(query.record().indexOf("Rk")).toDouble();
            q=query.value(query.record().indexOf("Qk")).toDouble();
            fprintf(fStd,"%03i%03i%6.4f%6.4f\n",sg,g,r,q);
        }
        if (query.value(query.record().indexOf("SubgroupPSRK")).toInt()>0){
            sg=query.value(query.record().indexOf("SubgroupPSRK")).toInt();
            g=query.value(query.record().indexOf("GroupPSRK")).toInt();
            r=query.value(query.record().indexOf("RkPSRK")).toDouble();
            q=query.value(query.record().indexOf("QkPSRK")).toDouble();
            fprintf(fPSRK,"%03i%03i%6.4f%6.4f\n",sg,g,r,q);
        }
        if (query.value(query.record().indexOf("SubgroupDortmund")).toInt()>0){
            sg=query.value(query.record().indexOf("SubgroupDortmund")).toInt();
            g=query.value(query.record().indexOf("GroupDortmund")).toInt();
            r=query.value(query.record().indexOf("RkDortmund")).toDouble();
            q=query.value(query.record().indexOf("QkDortmund")).toDouble();
            fprintf(fDort,"%03i%03i%6.4f%6.4f\n",sg,g,r,q);
        }
        if (query.value(query.record().indexOf("SubgroupNist")).toInt()>0){
            sg=query.value(query.record().indexOf("SubgroupNist")).toInt();
            g=query.value(query.record().indexOf("GroupNist")).toInt();
            r=query.value(query.record().indexOf("RkNist")).toDouble();
            q=query.value(query.record().indexOf("QkNist")).toDouble();
            fprintf(fNist,"%03i%03i%6.4f%6.4f\n",sg,g,r,q);
        }
    }
    fclose(fStd);
    fclose(fPSRK);
    fclose(fDort);
    fclose(fNist);
    query.prepare("SELECT * FROM UnifacStInteraction");
    query.exec();
    fStd=fopen("UnifacInterStd.txt","w+");
    while (query.next()){
            g1=query.value(query.record().indexOf("i")).toInt();
            g2=query.value(query.record().indexOf("j")).toInt();
            A12=query.value(query.record().indexOf("Aij")).toDouble();
            A21=query.value(query.record().indexOf("Aji")).toDouble();
            fprintf(fStd,"%03i%03i%010.4f%010.4f\n",g1,g2,A12,A21);
    }
    fclose(fStd);
    query.prepare("SELECT * FROM UnifacPSRKInteraction");
    query.exec();
    fPSRK=fopen("UnifacInterPSRK.txt","w+");
    while (query.next()){
            g1=query.value(query.record().indexOf("i")).toInt();
            g2=query.value(query.record().indexOf("j")).toInt();
            A12=query.value(query.record().indexOf("Aij")).toDouble();
            B12=query.value(query.record().indexOf("Bij")).toDouble();
            C12=query.value(query.record().indexOf("Cij")).toDouble();
            A21=query.value(query.record().indexOf("Aji")).toDouble();
            B21=query.value(query.record().indexOf("Bji")).toDouble();
            C21=query.value(query.record().indexOf("Cji")).toDouble();
            fprintf(fPSRK,"%03i%03i%010.3f%010.5f%010.7f%010.3f%010.5f%010.7f\n",g1,g2,A12,B12,C12,A21,B21,C21);
    }
    fclose(fPSRK);
    query.prepare("SELECT * FROM UnifacDortInteraction");
    query.exec();
    fDort=fopen("UnifacInterDort.txt","w+");
    while (query.next()){
            g1=query.value(query.record().indexOf("i")).toInt();
            g2=query.value(query.record().indexOf("j")).toInt();
            A12=query.value(query.record().indexOf("Aij")).toDouble();
            B12=query.value(query.record().indexOf("Bij")).toDouble();
            C12=query.value(query.record().indexOf("Cij")).toDouble();
            A21=query.value(query.record().indexOf("Aji")).toDouble();
            B21=query.value(query.record().indexOf("Bji")).toDouble();
            C21=query.value(query.record().indexOf("Cji")).toDouble();
            fprintf(fDort,"%03i%03i%010.3f%010.5f%010.7f%010.3f%010.5f%010.7f\n",g1,g2,A12,B12,C12,A21,B21,C21);
    }
    fclose(fDort);
    query.prepare("SELECT * FROM UnifacNistInteraction");
    query.exec();
    fNist=fopen("UnifacInterNist.txt","w+");
    while (query.next()){
            g1=query.value(query.record().indexOf("i")).toInt();
            g2=query.value(query.record().indexOf("j")).toInt();
            A12=query.value(query.record().indexOf("Aij")).toDouble();
            B12=query.value(query.record().indexOf("Bij")).toDouble();
            C12=query.value(query.record().indexOf("Cij")).toDouble();
            A21=query.value(query.record().indexOf("Aji")).toDouble();
            B21=query.value(query.record().indexOf("Bji")).toDouble();
            C21=query.value(query.record().indexOf("Cji")).toDouble();
            fprintf(fNist,"%03i%03i%010.3f%010.5f%010.7f%010.3f%010.5f%010.7f\n",g1,g2,A12,B12,C12,A21,B21,C21);
    }
    fclose(fNist);
}
