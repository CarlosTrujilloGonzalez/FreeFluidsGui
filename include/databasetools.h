/*
 * databasetools.h
 *
 *  Created on: 9/11/2015
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
#ifndef DATABASETOOLS
#define DATABASETOOLS

#include<string>
#include <QtSql/QSqlDatabase>
#include <QtSql>

#include "FFeosPure.h"
#include "FFphysprop.h"
//#include "FFbaseClasses.h"

//EOS conversion from QString to enumeration
void ConvertEosToEnumeration(const QString *eosModel,enum FF_EOS *eos);

//EOS conversion from std::string to enumeration
void ConvertEosToEnumeration2(const std::string *eosModel,enum FF_EOS *eos);

//EOS conversion from enumeration to std::string
void ConvertEnumerationToEos(const enum FF_EOS *eos, QString *eosModel);

//Get basic data for a SubstanceData structure from the database
void GetBasicData(int id,FF_SubstanceData *subsData,QSqlDatabase *db);


//This will recover the necessary information from the database, given product, eos and Cp0 correlation to be used
void GetEosData(const int *IdProduct,enum FF_EOS *eos,const int *IdEos,const int *IdCorrParam,QSqlDatabase *db,void *dataV,FF_Correlation *cp0);

//Get EOS data for a SubstanceData structure
void GetEOSData(int *eosType,FF_SubstanceData *subsData,QSqlDatabase *db);

void GetCorrDataByType(const int *IdProduct,const QString *type,QSqlDatabase *db,int *corrNum,double coef[]);


void GetCorrDataById(FF_Correlation *corr,QSqlDatabase *db);


//Adds a new eos to the database
void AddEosToDataBase(int idSubs,enum FF_EosType eosType,void *eosData,double *Tmin,double *Tmax, QString *description,QSqlDatabase *db);

//Adds a new correlation to the database
void AddCorrToDataBase(int idSubs,FF_Correlation *corr,double *Tmin,double *Tmax, QString *description,QSqlDatabase *db);

//Writes the Unifac information from the database to a file from where it can be extracted using C
void WriteUnifacToFile(QSqlDatabase *db);

#endif // DATABASETOOLS

