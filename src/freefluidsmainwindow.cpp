/*
 * freefluidsmainwindow.cpp
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
#include "freefluidsmainwindow.h"
#include "ui_freefluidsmainwindow.h"
#include <iostream>
#include <string>



FreeFluidsMainWindow::FreeFluidsMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FreeFluidsMainWindow)
{
    ui->setupUi(this);

    //***HERE BEGINS HAND WRITTEN CODE***
    //***********************************
    QString driver="QODBC";//driver to use. here ODBC
    QString database="Substances.dsn";//In ODBC the database must have been registered with a data source name
    db = QSqlDatabase::addDatabase(driver);
    db.setDatabaseName(database);
    if (!db.open()) {
        QMessageBox::critical(0, "No ODBC connection",
        "It has been impossible to connect to the ODBC database\n"
        "click Cancel to continue", QMessageBox::Cancel);
        db.removeDatabase(db.connectionName());
        driver="QSQLITE";
        database="Substances.db3";
        db = QSqlDatabase::addDatabase(driver);
        db.setDatabaseName(database);
        if (!db.open()) {
            QMessageBox::critical(0, "No SqLite database",
            "It has been impossible to connect to the SqLite database.\n"
            "Click Cancel to finish", QMessageBox::Cancel);
        }
    }
    //QueryModel(no editable) for holding the substances list
    subsListModel=new QSqlQueryModel(this);
    subsListModel->setQuery("SELECT Id,Name,MW from Products ORDER BY Name");
    //Data entry validators
    presBarValidator=new QDoubleValidator(0.0,10000.0,5,this);
    //presBarValidator->setNotation(QDoubleValidator::StandardNotation);
    tempCValidator=new QDoubleValidator(-273.15,4000.0,2,this);
    //tempCValidator->setNotation(QDoubleValidator::StandardNotation);
    subsCompleter= new QCompleter(this);
    subsCompleter->setModel(subsListModel);
    subsCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    subsCompleter->setCompletionColumn(1);
    subsCompleter->setCompletionMode(QCompleter::CompletionMode(1));
    //std::cout<<subsCompleter->completionModel()->

    //Common setup
    //************
    subs= new FF::Substance();//Creation of substance to hold up data
    subsData= new FF_SubstanceData;
    thSys= new FF::ThermoSystem(12);//Creation of substance to hold up data for 12 substances
    //fill with 0 the eos binary interaction parameters array
    for(int i=0;i<12;i++) for(int j=0;j<12;j++) for(int k=0;k<6;k++) pintParamEos[i][j][k]=0;

    //Combobox for substance selection model assignation
    ui->cbSubsCalcSelSubs->setCompleter(subsCompleter);
    ui->cbSubsCalcSelSubs->setModel(subsListModel);
    ui->cbSubsCalcSelSubs->setModelColumn(1);
    connect(ui->cbSubsCalcSelSubs,SIGNAL(currentIndexChanged(int)),this,SLOT(cbSubsCalcSelLoad(int)));//A new substance selection must update available EOS and Cp0


    //Substance calculation tab setup
    //*******************************

    //combobox for EOS selection, model and view assignation
    subsCalcEOSModel=new QSqlQueryModel(this);
    ui->cbSubsCalcSelEOS->setModel(subsCalcEOSModel);
    tvSubsCalcSelEOS = new QTableView(ui->cbSubsCalcSelEOS);
    tvSubsCalcSelEOS->verticalHeader()->hide();
    ui->cbSubsCalcSelEOS->setView(tvSubsCalcSelEOS);
    connect(ui->cbSubsCalcSelEOS,SIGNAL(currentIndexChanged(int)),this,SLOT(cbSubsCalcEosUpdate(int)));//We need to store the information of the selected eos

    //combobox for Cp0 selection, model and view assignation
    subsCalcCp0Model=new QSqlQueryModel(this);
    ui->cbSubsCalcSelCp0->setModel(subsCalcCp0Model);
    tvSubsCalcSelCp0 = new QTableView(ui->cbSubsCalcSelCp0);
    tvSubsCalcSelCp0->verticalHeader()->hide();
    ui->cbSubsCalcSelCp0->setView(tvSubsCalcSelCp0);
    connect(ui->cbSubsCalcSelCp0,SIGNAL(currentIndexChanged(int)),this,SLOT(cbSubsCalcCp0Update(int)));//We need to store the information of the selected cp0

    //Pressure and temperature selection
    ui->leSubsCalcPres->setValidator(presBarValidator);
    ui->leSubsCalcInitTemp->setValidator(tempCValidator);
    ui->leSubsCalcFinalTemp->setValidator(tempCValidator);

    //Button for calculation from T and P
    connect(ui->btnSubsCalcCalc,SIGNAL(clicked()),this,SLOT(twSubsCalcUpdate()));

    //Table widget for calculated data display
    QStringList subsCalcVertLabels;
    subsCalcVertLabels <<"T(C)"<<"Phase"<<"phi liq."<<"phi gas"<<"Z"<<"V(cm3/mol)"<<"rho(kgr/m3)"<<"H0(KJ/kgr)" <<"S0(KJ/kgr·K)"
                      <<"Cp0(KJ/kgr·K)"<<"H(KJ/kgr)"<<"U(KJ/kgr)"<<"S(KJ/kgr·K)"<<"Cp(KJ/kgr·K)"<<"Cv(KJ/kgr·K)"<<"S.S.(m/s)"<<"J.T.coeff(K/bar)"
                       <<"I.T.coeff(KJ/bar)"<<"(dP/dT)V(bar/K)"<<"(dP/dV)T(bar/m3)"<<"Vp(bar)"<<"rho liq. sat."<<"rho gas sat."<<"Hv sat(KJ/kgr)"<< "Arr"
                      <<"(dArr/dV)T"<<"(d2Arr/dV2)T"<<"(dArr/dT)V"<<"(d2Arr/dT2)V"<<"d2Arr/dTdV"<<"Vp corr."<<"Liq.rho corr."<<"Liq.visc.corr.(cps)";
    ui->twSubsCalc->setVerticalHeaderLabels(subsCalcVertLabels);
    for (int i=0;i<ui->twSubsCalc->columnCount();i++){
        for (int j=0;j<ui->twSubsCalc->rowCount();j++){
            ui->twSubsCalc->setItem(j,i,new QTableWidgetItem());
        }

    }

    //Button for alternative calculation
    connect(ui->btnSubsCalcAltCalc,SIGNAL(clicked()),this,SLOT(btnSubsCalcAltCalc()));

    //Button for table content export
    connect(ui->btnSubsCalcExport,SIGNAL(clicked()),this,SLOT(twSubsCalcExport()));

    //Button for substance export
    connect(ui->btnSubsCalcExportSubs,SIGNAL(clicked()),this,SLOT(btnSubsCalcExportSubs()));

    //Combobox for known varibables selection
    ui->cbSubsCalcKnownVars->addItems(QStringList () << "P,H"<<"P,U"<<"P,S"<< "rho,T"<< "rho,H");

    //Button for transfer from eos calculation to correlation calculation data tables
    connect(ui->btnSubsCalcTransfer,SIGNAL(clicked()),this,SLOT(btnSubsCalcTransfer()));


    //Substance tools tab setup
    //*************************

    //Table widget for  data display
    //QStringList subsToolsHorizLabels;
    //subsToolsHorizLabels <<"x / T"<<"y / Vp"<<"rho liq."<<"Cp liq."<<"S. of S."<<"Other";
    // ui->twSubsTools->setHorizontalHeaderLabels(subsToolsHorizLabels);
    for (int i=0;i<ui->twSubsTools->columnCount();i++){
        for (int j=0;j<ui->twSubsTools->rowCount();j++){
            ui->twSubsTools->setItem(j,i,new QTableWidgetItem());
        }
    }
    for (int i=0;i<ui->twSubsToolsCoefPrep->columnCount();i++){
        for (int j=0;j<ui->twSubsToolsCoefPrep->rowCount();j++){
            ui->twSubsToolsCoefPrep->setItem(j,i,new QTableWidgetItem());
        }
    }


    //combobox for correlation selection, model and view assignation
    subsToolsCorrModel=new QSqlQueryModel(this);
    ui->cbSubsToolsSelCorr->setModel(subsToolsCorrModel);
    tvSubsToolsSelCorr = new QTableView(ui->cbSubsToolsSelCorr);
    tvSubsToolsSelCorr->verticalHeader()->hide();
    //tvSubsToolsSelCorr->horizontalHeader()->hide();
    ui->cbSubsToolsSelCorr->setView(tvSubsToolsSelCorr);
    connect(ui->cbSubsToolsSelCorr,SIGNAL(currentIndexChanged(int)),this,SLOT(cbSubsToolsCorrLoad(int)));//it is necessary to register the selection made

    //Button for table filling from correlation
    connect(ui->btnSubsToolsFillTable,SIGNAL(clicked()),this,SLOT(btnSubsToolsFillTable()));

    //Button from table filling from external csv file
    connect(ui->btnSubsToolsImport,SIGNAL(clicked()),this,SLOT(twSubsToolsImport()));

    //Combobox for correlation selection for coefficients optimization
    connect(ui->cbSubsToolsSelOptCorr,SIGNAL(currentIndexChanged(int)),this,SLOT (cbSubsToolsSelOptCorr(int)));

    //Button for coefficients table, and partial errors, clearing
    connect(ui->btnSubsToolsClearCoefTable,SIGNAL(clicked()),this,SLOT(btnSubsToolsClearCoef()));

    //Button for correlation coefficients calculation
    connect(ui->btnSubsToolsFindCorr,SIGNAL(clicked()),this,SLOT(btnSubsToolsFindCorr()));

    //Button for EOS coefficients calculation
    connect(ui->btnSubsToolsFindEOS,SIGNAL(clicked()),this,SLOT(btnSubsToolsFindEOS()));

    //Button for EOS coefficients test
    connect(ui->btnSubsToolsTestEOS,SIGNAL(clicked()),this,SLOT(btnSubsToolsTestEOS()));

    //Button for table content clearing
    connect(ui->btnSubsToolsClearTable,SIGNAL(clicked()),this,SLOT(twSubsToolsClear()));

    //Button for table columns interchange
    connect(ui->btnSubsToolsExchangeCols,SIGNAL(clicked()),this,SLOT(twSubsToolsInterchange()));

    //Button for calculation on one table column
    connect(ui->btnSubsToolsDoOp,SIGNAL(clicked()),this,SLOT(twSubsToolsDoOperation()));

    //Button for adding EOS to database
    connect(ui->btnSubsToolsAddEos,SIGNAL(clicked()),this,SLOT(btnSubsToolAddEos()));

    //Button for adding correlation to database
    connect(ui->btnSubsToolsAddCorr,SIGNAL(clicked()),this,SLOT(btnSubsToolAddCorr()));

    //Mixture calculation tab setup
    //*****************************

    numSubs=0;
    //Table widget for mixture composition data
    ui->twMixComposition->setColumnWidth(0,34);
    ui->twMixComposition->setColumnWidth(1,180);
    ui->twMixComposition->setColumnWidth(2,0);
    QStringList mixCompHorLabels;
    mixCompHorLabels <<"Id"<<"Substance"<<"MW"<<"Quantity"<<"Mass frac."<<"Mole frac.";
    ui->twMixComposition->setHorizontalHeaderLabels(mixCompHorLabels);
    for (int i=0;i<ui->twMixComposition->columnCount();i++){
        for (int j=0;j<ui->twMixComposition->rowCount();j++){
            ui->twMixComposition->setItem(j,i,new QTableWidgetItem());
        }
    }

    //Button for adding substance to the mixture composition table
    connect(ui->btnMixCalcAddSubs,SIGNAL(clicked()),this,SLOT(twMixCompositionAdd()));

    //Button for mixture composition table content clearing
    connect(ui->btnMixCalcClearTableComp,SIGNAL(clicked()),this,SLOT(twMixCompositionClear()));

    //Button for mixture composition table mass and molar fractions calculation
    connect(ui->btnMixCalcFractions,SIGNAL(clicked()),this,SLOT(twMixCompositionCalcFract()));

    //Combobox for eos type selection
    ui->cbMixCalcEosTypeSelec->setToolTip("Select the type of eos to use");
    ui->cbMixCalcEosTypeSelec->setToolTipDuration(3000);
    ui->cbMixCalcEosTypeSelec->addItems(QStringList () <<"None"<<"Cubic, Peng-Robinson"<<"Cubic, Soave-Redlich-Kwong"<<"PCSAFT"<< "Span-Wagner");
    connect(ui->cbMixCalcEosTypeSelec,SIGNAL(currentIndexChanged(int)),this,SLOT(cbMixCalcEosTypeLoad(int)));//A new EOS type selection must update available EOS

    //combobox for mixing rule selection
    ui->cbMixCalcMixRuleSelec->addItems(QStringList ()<<"None"<<"Van der Waals"<<"Panagiotopoulos and Reid"<<"Mathias, Klotz and Prausnitz");
    connect(ui->cbMixCalcMixRuleSelec,SIGNAL(currentIndexChanged(int)),this,SLOT(cbMixCalcMixRuleLoad(int)));//Mixing rule selection is stored

    //Combobox for liquid phase model selection
    ui->cbMixCalcLiqModelSelec->setToolTip("Select the model to use for the liquid phase");
    ui->cbMixCalcLiqModelSelec->setToolTipDuration(3000);
    ui->cbMixCalcLiqModelSelec->addItems(QStringList () <<"Eos"<<"Activity");
    //connect(ui->cbMixCalcEosTypeSelec,SIGNAL(currentIndexChanged(int)),this,SLOT(cbMixCalcEosTypeLoad(int)));//If eos is selected, the activity model should be disabled

    //Combobox for activity model selection
    ui->cbMixCalcActModelSelec->setToolTip("Select the model to use for activity calculation");
    ui->cbMixCalcActModelSelec->setToolTipDuration(3000);
    ui->cbMixCalcActModelSelec->addItems(QStringList () <<"Wilson"<<"NRTL"<<"UNIQUAC");

    //Combobox for reference fugacity selection
    ui->cbMixCalcRefPhiSelec->setToolTip("Select the reference fugacity calculation to use with activity models");
    ui->cbMixCalcRefPhiSelec->setToolTipDuration(3000);
    ui->cbMixCalcRefPhiSelec->addItems(QStringList () <<"Eos"<<"Vp");

    //combobox for EOS selection, model and view assignation for substances
    for (int i=0;i<6;i++){
    mixCalcEOSModel[i] = new QSqlQueryModel(this);
    tvMixCalcSelEOS[i] = new QTableView(this);
    tvMixCalcSelEOS[i]->verticalHeader()->hide();
    }
    ui->cbMixCalcEosSelec1->setModel(mixCalcEOSModel[0]);
    ui->cbMixCalcEosSelec2->setModel(mixCalcEOSModel[1]);
    ui->cbMixCalcEosSelec3->setModel(mixCalcEOSModel[2]);
    ui->cbMixCalcEosSelec4->setModel(mixCalcEOSModel[3]);
    ui->cbMixCalcEosSelec5->setModel(mixCalcEOSModel[4]);
    ui->cbMixCalcEosSelec6->setModel(mixCalcEOSModel[5]);
    //tvMixCalcSelEOS[0] = new QTableView(ui->cbMixCalcEosSelec1 );
    ui->cbMixCalcEosSelec1->setView(tvMixCalcSelEOS[0]);
    ui->cbMixCalcEosSelec2->setView(tvMixCalcSelEOS[1]);
    ui->cbMixCalcEosSelec3->setView(tvMixCalcSelEOS[2]);
    ui->cbMixCalcEosSelec4->setView(tvMixCalcSelEOS[3]);
    ui->cbMixCalcEosSelec5->setView(tvMixCalcSelEOS[4]);
    ui->cbMixCalcEosSelec6->setView(tvMixCalcSelEOS[5]);

    //combobox for Cp0 selection, model and view assignation for substances
    for (int i=0;i<6;i++){
    mixCalcCp0Model[i] = new QSqlQueryModel(this);
    tvMixCalcSelCp0[i] = new QTableView(this);
    tvMixCalcSelCp0[i]->verticalHeader()->hide();
    }
    ui->cbMixCalcCp0Selec1->setModel(mixCalcCp0Model[0]);
    ui->cbMixCalcCp0Selec2->setModel(mixCalcCp0Model[1]);
    ui->cbMixCalcCp0Selec3->setModel(mixCalcCp0Model[2]);
    ui->cbMixCalcCp0Selec4->setModel(mixCalcCp0Model[3]);
    ui->cbMixCalcCp0Selec5->setModel(mixCalcCp0Model[4]);
    ui->cbMixCalcCp0Selec6->setModel(mixCalcCp0Model[5]);
    ui->cbMixCalcCp0Selec1->setView(tvMixCalcSelCp0[0]);
    ui->cbMixCalcCp0Selec2->setView(tvMixCalcSelCp0[1]);
    ui->cbMixCalcCp0Selec3->setView(tvMixCalcSelCp0[2]);
    ui->cbMixCalcCp0Selec4->setView(tvMixCalcSelCp0[3]);
    ui->cbMixCalcCp0Selec5->setView(tvMixCalcSelCp0[4]);
    ui->cbMixCalcCp0Selec6->setView(tvMixCalcSelCp0[5]);

    //tablewidget for pair i,j selection and indication
    connect(ui->twMixPairSel,SIGNAL(cellClicked(int,int)),this,SLOT(twMixIntParamEosDisplay(int,int)));

    //combobox for interaction param selection
    mixIntParamSelModel = new QSqlQueryModel(this);
    tvMixIntParamSel = new QTableView(this);
    ui->cbMixIntParamSel->setModel(mixIntParamSelModel);
    ui->cbMixIntParamSel->setView(tvMixIntParamSel);
    tvMixIntParamSel->horizontalHeader()->setDefaultSectionSize(68);
    connect(ui->cbMixIntParamSel,SIGNAL(currentIndexChanged(int)),this,SLOT(twMixIntParamEosFill(int)));

    //tablewidget for selected pair eos interaction display
    ui->twMixIntParamEos->verticalHeader()->setVisible(true);
    ui->twMixIntParamEos->setColumnWidth(7,195);
    ui->twMixIntParamEos->setHorizontalHeaderLabels(QStringList()<<"k1/A"<<"k2/B"<<"k3/C"<<"l1/D"<<"l2/E"<<"l3/F"<<"Eq."<<"Description");
    ui->twMixIntParamEos->setVerticalHeaderLabels(QStringList()<<"i,j"<<"j,i");
    for (int i=0;i<ui->twMixIntParamEos->columnCount();i++){
        for (int j=0;j<ui->twMixIntParamEos->rowCount();j++){
            ui->twMixIntParamEos->setItem(j,i,new QTableWidgetItem());
        }
    }
    connect(ui->twMixIntParamEos,SIGNAL(cellChanged(int,int)),this,SLOT(twMixIntParamEosUpdate(int,int)));


    //Button for mixture calculation from T and P
     ui->btnMixCalcCalc->setToolTip("Calculation is done at the given global composition at specified T and P, and written to the table");
     ui->btnMixCalcCalc->setToolTipDuration(4000);
     connect(ui->btnMixCalcCalc,SIGNAL(clicked()),this,SLOT(twMixCalcUpdate()));

    //Button for mixture bubble T calculation at given P
          connect(ui->btnMixCalcBubbleT,SIGNAL(clicked()),this,SLOT(twMixCalcBubbleT()));

    //Button for mixture dew T calculation at given P
          connect(ui->btnMixCalcDewT,SIGNAL(clicked()),this,SLOT(twMixCalcDewT()));

    //Button for mixture bubble P calculation at given P
          connect(ui->btnMixCalcBubbleP,SIGNAL(clicked()),this,SLOT(twMixCalcBubbleP()));

    //Button for mixture dew T calculation at given P
          connect(ui->btnMixCalcDewP,SIGNAL(clicked()),this,SLOT(twMixCalcDewP()));

    //Button for mixture dew T calculation at given P
          connect(ui->btnMixCalcPenvelope,SIGNAL(clicked()),this,SLOT(twMixCalcPenvelope()));

    //Mixture results tab setup
    //*************************

    //Table widget for calculated mixture data display
    QStringList mixCalcHorLabels;
    QStringList mixCalcVerLabels;
    mixCalcHorLabels <<"Total"<<"Gas"<<"Liquid 1"<<"Liquid 2";
    mixCalcVerLabels <<"MW"<<"P (bara)"<<"T(C)"<<"Phase fraction"<<"phi liq."<<"phi gas"<<"Z"<<"V(cm3/mol)"<<"rho(kgr/m3)"<<"H0(KJ/kgr)" <<"S0(KJ/kgr·K)"
                     <<"Cp0(KJ/kgr·K)"<<"Cv0(KJ/kgr·K)"<<"H(KJ/kgr)"<<"U(KJ/kgr)"<<"S(KJ/kgr·K)"<<"Cp(KJ/kgr·K)"<<"Cv(KJ/kgr·K)"<<"S.S.(m/s)"<<"J.T.coeff(K/bar)"
                     <<"I.T.coeff(KJ/bar)"<<"(dP/dT)V(bar/K)"<<"(dP/dV)T(bar/m3)"<< "Arr"<<"(dArr/dV)T"<<"(d2Arr/dV2)T"<<"(dArr/dT)V"<<"(d2Arr/dT2)V"
                     <<"d2Arr/dTdV"<<"Mol fract.1"<<"Mol fract.2"<<"Mol fract.3"<<"Mol fract.4"<<"Mol fract.5"<<"Mol fract.6"<<"Mass fract.1"<<"Mass fract.2"
                     <<"Mass fract.3"<<"Mass fract.4"<<"Mass fract.5"<<"Mass fract.6"<<"Phi 1"<<"Phi 2"<<"Phi 3"<<"Phi 4"<<"Phi 5"<<"Phi 6";
    ui->twMixCalc->setHorizontalHeaderLabels(mixCalcHorLabels);
    ui->twMixCalc->setVerticalHeaderLabels(mixCalcVerLabels);
    for (int i=0;i<ui->twMixCalc->columnCount();i++)for (int j=0;j<ui->twMixCalc->rowCount();j++) ui->twMixCalc->setItem(j,i,new QTableWidgetItem());

    //Table widget for envelope display
    ui->twMixEnvelope->setHorizontalHeaderLabels(QStringList()<<"Base"<<"Bubble"<<"gas"<<"Dew"<<"liquid");
    for (int i=0;i<ui->twMixEnvelope->columnCount();i++)for (int j=0;j<ui->twMixEnvelope->rowCount();j++) ui->twMixEnvelope->setItem(j,i,new QTableWidgetItem());
}

FreeFluidsMainWindow::~FreeFluidsMainWindow()
{
    delete ui;
    delete subs;
    delete thSys;
}

//Common functions
//****************

//Slot for loading the existing EOS and correlations for the selected substance
void FreeFluidsMainWindow::cbSubsCalcSelLoad(int position)
{
    //Delete old substance,create a new one, and clear screen
    delete subs;
    delete subsData;
    subs=new FF::Substance();
    subsData= new FF_SubstanceData;
    subs->id=subsListModel->record(position).value("Id").toInt();
    subsData->id=subsListModel->record(position).value("Id").toInt();
    ui->leSubsCalcCp0->setText("");
    ui->leSubsCalcCubic->setText("");
    ui->leSubsCalcSaft->setText("");
    ui->leSubsCalcSW->setText("");
    ui->chbSubsCalcCp0->setChecked(false);
    ui->chbSubsCalcCubic->setChecked(false);
    ui->chbSubsCalcSaft->setChecked(false);
    ui->chbSubsCalcSW->setChecked(false);
    ui->leSubsToolsLdensCorr->setText("");
    ui->leSubsToolsVpCorr->setText("");
    ui->leSubsToolsHvCorr->setText("");
    ui->leSubsToolsLCpCorr->setText("");
    ui->leSubsToolsLviscCorr->setText("");
    ui->leSubsToolsLthCCorr->setText("");
    ui->leSubsToolsLsurfTCorr->setText("");
    ui->leSubsToolsGdensCorr->setText("");
    ui->leSubsToolsGviscCorr->setText("");
    ui->leSubsToolsGthCCorr->setText("");
    ui->leSubsToolsSdensCorr->setText("");
    ui->leSubsToolsSCpCorr->setText("");
    ui->chbSubsToolsLdens->setChecked(false);
    ui->chbSubsToolsVp->setChecked(false);
    ui->chbSubsToolsHv->setChecked(false);
    ui->chbSubsToolsLCp->setChecked(false);
    ui->chbSubsToolsLvisc->setChecked(false);
    ui->chbSubsToolsLthC->setChecked(false);
    ui->chbSubsToolsLsurfT->setChecked(false);
    ui->chbSubsToolsGdens->setChecked(false);
    ui->chbSubsToolsGvisc->setChecked(false);
    ui->chbSubsToolsGthC->setChecked(false);
    ui->chbSubsToolsSdens->setChecked(false);
    ui->chbSubsToolsSCp->setChecked(false);
    ui->leSubsToolsLdensTref->setText("0");
    ui->chbSubsToolsLdensRa->setChecked(false);
    ui->leSubsToolsLdensDref->setText("0");
    ui->chbSubsToolsVpAmbrose->setChecked(false);
    ui->chbSubsToolsVpRiedel->setChecked(false);
    ui->leSubsToolsVpTref->setText("0");

    //****************
    //================
    GetGeneralData(subs,&db);//we charge in substance the general data
    GetBasicData(subsData->id,subsData,&db);
    //printf("Tc:%f\n",subsData->baseProp.Tc);


    if (subsData->baseProp.MW > 0) ui->leSubsToolsMW->setText(QString::number(subsData->baseProp.MW));//and now from substance to screen
    if (subsData->baseProp.Tc>0) ui->leSubsToolsTc->setText(QString::number(subsData->baseProp.Tc));
    if (subsData->baseProp.Pc > 0) ui->leSubsToolsPc->setText(QString::number(subsData->baseProp.Pc));
    if ((subsData->baseProp.Vc > 0)&&(subsData->baseProp.MW > 0)) ui->leSubsToolsRc->setText(QString::number(subsData->baseProp.MW/subsData->baseProp.Vc*1e-3));
    else if ((subsData->baseProp.Zc > 0)&&(subsData->baseProp.MW > 0)) ui->leSubsToolsRc->setText(QString::number(subsData->baseProp.MW*subsData->baseProp.Pc/
            (R*subsData->baseProp.Zc*subsData->baseProp.Tc*1e3)));
    if (subsData->baseProp.Zc > 0) ui->leSubsToolsZc->setText(QString::number(subsData->baseProp.Zc));
    if (subsData->baseProp.w > 0) ui->leSubsToolsW->setText(QString::number(subsData->baseProp.w));
    if (subs->VdWV>0) ui->leSubsToolsVdWV->setText(QString::number(subs->VdWV));
    if (subsData->lDens.y>0){
        ui->leSubsToolsLdensTref->setText(QString::number(subsData->lDens.x));
        ui->leSubsToolsLdensDref->setText(QString::number(subsData->lDens.y));
    }
    if (subsData->baseProp.Tb > 0){
        ui->leSubsToolsVpTref->setText(QString::number(subsData->baseProp.Tb));
        ui->leSubsToolsVpPref ->setText("101325");
    }
    else if (subsData->vp.y>0){
        ui->leSubsToolsVpTref->setText(QString::number(subsData->vp.x));
        ui->leSubsToolsVpPref ->setText(QString::number(subsData->vp.y));
    }

    //Charge EOS, Cp0, and correlations comboboxes with the options available for the substance
    QSqlQuery queryEos,queryCp0,queryCorr;
    queryEos.prepare("SELECT EosParam.Eos As Eos,EosParam.Description As Description,EosParam.Id As Id,EosParam.Tmin as Tmin,EosParam.Tmax as Tmax,Eos.Type As Type "
                     "FROM EosParam INNER JOIN Eos ON EosParam.Eos=Eos.Eos WHERE ((IdProduct)=?) ORDER BY EosParam.Eos");
    queryEos.addBindValue(subs->id);
    queryEos.exec();
    subsCalcEOSModel->setQuery(queryEos);
    tvSubsCalcSelEOS->setColumnHidden(2,true);
    tvSubsCalcSelEOS->setColumnWidth(0,78);
    tvSubsCalcSelEOS->setColumnWidth(1,220);
    tvSubsCalcSelEOS->setColumnWidth(3,46);
    tvSubsCalcSelEOS->setColumnWidth(4,46);
    tvSubsCalcSelEOS->setColumnWidth(5,72);
    //tvSubsCalcSelEOS->resizeColumnsToContents();
    queryCp0.prepare("SELECT CorrelationEquations.Equation AS Equation, CorrelationParam.Reference AS Reference, CorrelationParam.Id AS Id FROM PhysProp "
                     "INNER JOIN (CorrelationEquations INNER JOIN (Correlations INNER JOIN CorrelationParam ON "
                     "Correlations.Number = CorrelationParam.NumCorrelation) ON CorrelationEquations.Id = "
                     "Correlations.IdEquation) ON PhysProp.Id = Correlations.IdPhysProp WHERE (((CorrelationParam.IdProduct)=?) "
                     "AND ((PhysProp.Property)=?)) ORDER BY Equation");

    queryCp0.addBindValue(subsListModel->record(position).value("Id"));
    queryCp0.addBindValue("Cp0");
    queryCp0.exec();
    subsCalcCp0Model->setQuery(queryCp0);
    tvSubsCalcSelCp0->setColumnHidden(2,true);
    tvSubsCalcSelCp0->resizeColumnsToContents();
    //ui->lineEdit->setText(subsListModel->record(position).value("Name").toString());
    //Query for loading available correlation parameters for the selected substance
    queryCorr.prepare("SELECT PhysProp.Property AS Property, CorrelationEquations.Equation AS Equation, CorrelationParam.Reference AS Reference, "
                      "CorrelationParam.Tmin AS Tmin, CorrelationParam.Tmax AS Tmax, CorrelationParam.Id AS Id "
                      "FROM PhysProp INNER JOIN (CorrelationEquations INNER JOIN (Correlations INNER JOIN CorrelationParam ON "
                      "Correlations.Number = CorrelationParam.NumCorrelation) ON CorrelationEquations.Id = Correlations.IdEquation) "
                      "ON PhysProp.Id = Correlations.IdPhysProp WHERE (((CorrelationParam.IdProduct)=?))ORDER BY Property;");

    queryCorr.addBindValue(subsListModel->record(position).value("Id"));
    queryCorr.exec();
    subsToolsCorrModel->setQuery(queryCorr);
    tvSubsToolsSelCorr->setColumnHidden(5,true);
    tvSubsToolsSelCorr->resizeColumnsToContents();
}
//Substance calculation tab functions
//***********************************


//******************************
//==============================

//Slot for eos data update in the substance
void FreeFluidsMainWindow::cbSubsCalcEosUpdate(int position){
    std::string eosTypeString;
    eosTypeString=subsCalcEOSModel->record(position).value("Type").toString().toStdString();
    std::string eosInfo=subsCalcEOSModel->record(position).value("Description").toString().toStdString();
    eosInfo=eosInfo + "  Tmin:"+subsCalcEOSModel->record(position).value("Tmin").toString().toStdString()+"  Tmax:"+subsCalcEOSModel->record(position).value("Tmax").toString().toStdString();
    if((eosTypeString=="Cubic PR")||(eosTypeString=="Cubic SRK")){
        subs->eosType=FF_CubicType;
        subs->cubicData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        subsData->cubicData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        ui->leSubsCalcCubic->setText(QString::fromStdString(eosInfo));
        ui->chbSubsCalcCubic->setChecked(true);
    }
    else if (eosTypeString=="SAFT"){
        subs->eosType=FF_SAFTtype;
        subs->saftData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        subsData->saftData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        ui->leSubsCalcSaft->setText(QString::fromStdString(eosInfo));
        ui->chbSubsCalcSaft->setChecked(true);
    }
    else if (eosTypeString=="Multiparameter"){
        subs->eosType=FF_SWtype;
        subs->swData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        subsData->swData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        ui->leSubsCalcSW->setText(QString::fromStdString(eosInfo));
        ui->chbSubsCalcSW->setChecked(true);
    }
    GetEosData2(&subs->eosType,subs,&db);
    GetEOSData(&subs->eosType,subsData,&db);
    //std::cout<<"Cubic eos:"<<subs->cubicData.id<<" Saft eos:"<<subs->saftData.id<<" SW eos:"<<subs->swData.id<<std::endl;
    //printf("Cubic k1:%f\n",subsData->cubicData.k1);
    //printf("SW n[0] :%f\n",subsData->swData.n[0]);
}


//***************************
//===========================

//Slot for cp0 data update in the substance
void FreeFluidsMainWindow::cbSubsCalcCp0Update(int position){
    subs->cp0Corr.id=subsCalcCp0Model->record(position).value("Id").toInt();
    GetCorrDataById2(&subs->cp0Corr,&db);

    subsData->cp0Corr.id=subsCalcCp0Model->record(position).value("Id").toInt();
    GetCorrDataById2(&subsData->cp0Corr,&db);
    //printf("cp0 A:%f\n",subsData->cp0Corr.coef[0]);

    ui->leSubsCalcCp0->setText(subsCalcCp0Model->record(position).value("Reference").toString());
    ui->chbSubsCalcCp0->setChecked(true);
    ui->leSubsToolsCp0Corr->setText(subsCalcCp0Model->record(position).value("Reference").toString());

    //std::cout<<"Cp0 id:"<<subs->cp0Corr.id<<" form:"<<subs->cp0Corr.form<<" A:"<<subs->cp0Corr.coef[0]<<std::endl;
}

//Slot for substance calculation, and display in table
void FreeFluidsMainWindow::twSubsCalcUpdate()
{
    int i,j;//the loop variables
    for (i=0;i<ui->twSubsCalc->rowCount();i++) for (j=0;j<ui->twSubsCalc->columnCount();j++) ui->twSubsCalc->item(i,j)->setText("");//we clear the content

    //if (!((subsId>0)&&(eosId>0))) return;//Without substance or eos calculation is impossible
    //std::cout<<subsId<<" "<<eosId<<" "<<cp0Id<<" "<<eosModel.toStdString()<<std::endl;

    //And now we make calculations
    FF_ThermoProperties th0,thR,thVp;//here will be stored the result of the calculations
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will recive here the state of the calculation
    double MW;//molecular weight
    double Tb;//boiling temperature
    thR.P=1e5*ui->leSubsCalcPres->text().toDouble();//we read the selected pressure
    double initT=ui->leSubsCalcInitTemp->text().toDouble()+273.15;
    double finalT=ui->leSubsCalcFinalTemp->text().toDouble()+273.15;
    double Tincrement=(finalT-initT)/10;//temperature increments between calculations
    double answerL[3],answerG[3],answerLVp[3],answerGVp[3];
    double phiL,phiG;
    QString phase;
    double Z;
    double Vp;
    double Hv;
    double ArrDerivatives[6];
    FF_CubicParam param;

    if(subs->eosType==FF_SAFTtype){
        MW=subsData->saftData.MW;
        FF_TbEOS(&subs->eosType,&thR.P,&subsData->saftData,&Tb);//boiling point calculation
    }
    else if(subs->eosType==FF_SWtype){
        MW=subsData->swData.MW;
        FF_TbEOS(&subs->eosType,&thR.P,&subsData->swData,&Tb);//boiling point calculation
    }
    else{
        MW=subsData->cubicData.MW;
        FF_TbEOS(&subs->eosType,&thR.P,&subsData->cubicData,&Tb);//boiling point calculation
    }
    thR.MW=MW;
    ui->leSubsCalcMW->setText(QString::number(MW));
    ui->leSubsCalcTb->setText(QString::number(Tb-273.15));
    for (i=0;i<11;i++) {
        th0.T=thVp.T=thR.T=initT+i*Tincrement;
        thR.P=1e5*ui->leSubsCalcPres->text().toDouble();//we read the selected pressure. Necessary to do each time, because it is changed
        if(subs->eosType==FF_SAFTtype){
            FF_VfromTPeos(&subs->eosType,&thR.T,&thR.P,&subsData->saftData,&option,answerL,answerG,&state);//Volume, Arr, Z and fugacity coeff. retrieval
            //return;
            phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
            phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
            if (state=='f') phase="Calc.fail";
            else if ((state=='l')||(state=='g')||(state=='b')) phase="Unique";
            else if (state=='L') phase="Liquid";
            else if (state=='G') phase="Gas";
            else if (state=='E') phase="Equilibrium";
            if ((state=='l')||(state=='L')||(state=='b')){
                thR.V=answerL[0];
                Z=answerL[2];
            }
            else if ((state=='g')||(state=='G')){
                thR.V=answerG[0];
                Z=answerG[2];
            }
            //printf("state:%c\n",state);
             th0.V=thR.V;
            FF_IdealThermoEOS(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&th0);
            FF_ThermoEOS(&subs->eosType,&subsData->saftData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&thR);
            FF_ArrDerPCSAFT(&thR.T,&thR.V,&subsData->saftData,ArrDerivatives);
            if (ui->chbSubsCalcSatProp->isChecked()) FF_VpEOS(&subs->eosType,&thR.T,&subsData->saftData,&Vp);//Vapor pressure calculation
            else Vp=0;
            if ((Vp>0) && (Vp<subsData->saftData.Pc)){//If Vp has been calculated as is lower than Pc
                FF_VfromTPeos(&subs->eosType,&thR.T,&Vp,&subsData->saftData,&option,answerLVp,answerGVp,&state);//We calculate liquid and gas volumes at Vp
                thVp.T=thR.T;
                thVp.V=answerGVp[0];
                FF_ExtResidualThermoEOS(&subs->eosType,&subsData->saftData,&thVp);//with the gas volume we calculate the residual thermo properties
                Hv=thVp.H;
                thVp.V=answerLVp[0];
                FF_ExtResidualThermoEOS(&subs->eosType,&subsData->saftData,&thVp);//with the liquid volume we calculate the residual thermo properties
                Hv=Hv-thVp.H;//vaporization enthalpy is the difference
            }
        }
        else if(subs->eosType==FF_SWtype){
            FF_VfromTPeos(&subs->eosType,&thR.T,&thR.P,&subsData->swData,&option,answerL,answerG,&state);//Volume, Arr, Z and fugacity coeff. retrieval
            phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
            phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
            if (state=='f') phase="Calc.fail";
            else if ((state=='l')||(state=='g')||(state=='b')) phase="Unique";
            else if (state=='L') phase="Liquid";
            else if (state=='G') phase="Gas";
            else if (state=='E') phase="Equilibrium";
            if ((state=='l')||(state=='L')||(state=='b')){
                thR.V=answerL[0];
                Z=answerL[2];
            }
            else if ((state=='g')||(state=='G')){
                thR.V=answerG[0];
                Z=answerG[2];
            }
            //printf("state:%c\n",state);
            th0.V=thR.V;
            if (subsData->swData.eos==FF_IAPWS95) FF_IdealThermoWater(&th0);
            else FF_IdealThermoEOS(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&th0);
            FF_ThermoEOS(&subs->eosType,&subsData->swData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&thR);
            FF_ArrDerSWTV(&thR.T,&thR.V,&subsData->swData,ArrDerivatives);
            if (ui->chbSubsCalcSatProp->isChecked()) FF_VpEOS(&subs->eosType,&thR.T,&subsData->swData,&Vp);//Vapor pressure calculation
            else Vp=0;
            if ((Vp>0) && (Vp<subsData->swData.Pc)){//If Vp has been calculated as is lower than Pc
                FF_VfromTPeos(&subs->eosType,&thR.T,&Vp,&subsData->swData,&option,answerLVp,answerGVp,&state);//We calculate liquid and gas volumes at Vp
                thVp.T=thR.T;
                thVp.V=answerGVp[0];
                FF_ExtResidualThermoEOS(&subs->eosType,&subsData->swData,&thVp);//with the gas volume we calculate the residual thermo properties
                Hv=thVp.H;
                thVp.V=answerLVp[0];
                FF_ExtResidualThermoEOS(&subs->eosType,&subsData->swData,&thVp);//with the liquid volume we calculate the residual thermo properties
                Hv=Hv-thVp.H;//vaporization enthalpy is the difference
            }
        }
        else{
            FF_VfromTPeos(&subs->eosType,&thR.T,&thR.P,&subsData->cubicData,&option,answerL,answerG,&state);//Volume, Arr, Z and fugacity coeff. retrieval
            //printf("T:%f P:%f Vl:%f Zl:%f Vg:%f Zg:%f\n",thR.T,thR.P,answerL[0],answerL[2],answerG[0],answerG[2]);
            phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
            phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
            if (state=='f') phase="Calc.fail";
            else if ((state=='l')||(state=='g')||(state=='b')) phase="Unique";
            else if (state=='L') phase="Liquid";
            else if (state=='G') phase="Gas";
            else if (state=='E') phase="Equilibrium";
            if ((state=='l')||(state=='L')||(state=='b')){
                thR.V=answerL[0];
                Z=answerL[2];
            }
            else if ((state=='g')||(state=='G')){
                thR.V=answerG[0];
                Z=answerG[2];
            }
            //printf("state:%c\n",state);
            th0.V=thR.V;
            FF_IdealThermoEOS(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&th0);
            FF_ThermoEOS(&subs->eosType,&subsData->cubicData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&thR);
            FF_FixedParamCubic(&subsData->cubicData,&param);
            FF_ThetaDerivCubic(&thR.T,&subsData->cubicData,&param);
            FF_ArrDerCubic(&thR.T,&thR.V,&param,ArrDerivatives);
            if (ui->chbSubsCalcSatProp->isChecked()) FF_VpEOS(&subs->eosType,&thR.T,&subsData->cubicData,&Vp);//Vapor pressure calculation
            else Vp=0;
            if ((Vp>0) && (Vp<subsData->cubicData.Pc)){//If Vp has been calculated as is lower than Pc
                FF_VfromTPeos(&subs->eosType,&thR.T,&Vp,&subsData->cubicData,&option,answerLVp,answerGVp,&state);//We calculate liquid and gas volumes at Vp
                thVp.T=thR.T;
                thVp.V=answerGVp[0];
                FF_ExtResidualThermoEOS(&subs->eosType,&subsData->cubicData,&thVp);//with the gas volume we calculate the residual thermo properties
                Hv=thVp.H;
                thVp.V=answerLVp[0];
                FF_ExtResidualThermoEOS(&subs->eosType,&subsData->cubicData,&thVp);//with the liquid volume we calculate the residual thermo properties
                Hv=Hv-thVp.H;//vaporization enthalpy is the difference
            }
        }

        //here we write the results to the table
        ui->twSubsCalc->item(0,i)->setText(QString::number(thR.T-273.15));
        ui->twSubsCalc->item(1,i)->setText(phase);
        ui->twSubsCalc->item(2,i)->setText(QString::number(phiL));
        ui->twSubsCalc->item(3,i)->setText(QString::number(phiG));
        ui->twSubsCalc->item(4,i)->setText(QString::number(Z));//Z
        ui->twSubsCalc->item(5,i)->setText(QString::number(thR.V*1e6));//Molar volume cm3/mol
        ui->twSubsCalc->item(6,i)->setText(QString::number(MW/thR.V/1000));//rho kgr/m3
        ui->twSubsCalc->item(7,i)->setText(QString::number(th0.H/MW));//H0 KJ/kgr
        ui->twSubsCalc->item(8,i)->setText(QString::number(th0.S/MW));
        ui->twSubsCalc->item(9,i)->setText(QString::number(th0.Cp/MW));//Cp0 KJ/kgr·K
        ui->twSubsCalc->item(10,i)->setText(QString::number(thR.H/MW));
        ui->twSubsCalc->item(11,i)->setText(QString::number(thR.U/MW));
        ui->twSubsCalc->item(12,i)->setText(QString::number(thR.S/MW));
        ui->twSubsCalc->item(13,i)->setText(QString::number(thR.Cp/MW));
        ui->twSubsCalc->item(14,i)->setText(QString::number(thR.Cv/MW));
        ui->twSubsCalc->item(15,i)->setText(QString::number(thR.SS));
        ui->twSubsCalc->item(16,i)->setText(QString::number(thR.JT*1e5));
        ui->twSubsCalc->item(17,i)->setText(QString::number(thR.IT*1e2));
        ui->twSubsCalc->item(18,i)->setText(QString::number(thR.dP_dT*1e-5));
        ui->twSubsCalc->item(19,i)->setText(QString::number(thR.dP_dV*1e-5));
        if (Vp>0){
            ui->twSubsCalc->item(20,i)->setText(QString::number(Vp/100000));//Vapor pressure bar
            ui->twSubsCalc->item(21,i)->setText(QString::number(MW/answerLVp[0]/1000));//liquid rho at Vp kgr/m3
            ui->twSubsCalc->item(22,i)->setText(QString::number(MW/answerGVp[0]/1000));//gas rho at Vp kgr/m3
            ui->twSubsCalc->item(23,i)->setText(QString::number(Hv/MW));//Saturated vaporization enthalpy
        }
        ui->twSubsCalc->item(24,i)->setText(QString::number(ArrDerivatives[0]));
        ui->twSubsCalc->item(25,i)->setText(QString::number(ArrDerivatives[1]));
        ui->twSubsCalc->item(26,i)->setText(QString::number(ArrDerivatives[2]));
        ui->twSubsCalc->item(27,i)->setText(QString::number(ArrDerivatives[3]));
        ui->twSubsCalc->item(28,i)->setText(QString::number(ArrDerivatives[4]));
        ui->twSubsCalc->item(29,i)->setText(QString::number(ArrDerivatives[5]));
    }

    //Now we add some phys.prop. calculations made by correlations
    QString type;
    int nColumns=ui->twSubsCalc->columnCount();
    double x[nColumns];
    double y[nColumns];
    for (i=0;i<11;i++) {
        x[i]=initT+i*Tincrement;
    }
    type="Vp";
    GetCorrDataByType(&subs->id,&type,&db,&subs->vpCorr.form,subs->vpCorr.coef);
    //std::cout<<corrEq<<" "<<corrCoef[0]<<" "<<corrCoef[1]<<" "<<corrCoef[2]<<" "<<corrCoef[3]<<" "<<corrCoef[4]<<" "<<corrCoef[5]<<std::endl;

    FF_PhysPropCorr(&subs->vpCorr.form,subs->vpCorr.coef,&MW,&nColumns,x,y);
    for (i=0;i<11;i++) {
        ui->twSubsCalc->item(30,i)->setText(QString::number(y[i]*1e-5));
    }
    type="Ldens";
    GetCorrDataByType(&subs->id,&type,&db,&subs->lDensCorr.form,subs->lDensCorr.coef);
    FF_PhysPropCorr(&subs->lDensCorr.form,subs->lDensCorr.coef,&MW,&nColumns,x,y);
    for (i=0;i<11;i++) {
        ui->twSubsCalc->item(31,i)->setText(QString::number(y[i]));
    }
}

//Slot for alternative calculation (not from T and P)
void FreeFluidsMainWindow::btnSubsCalcAltCalc(){


    //And now we make calculations
    FF_ThermoProperties thR;//here will be stored the result of the calculations
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='b';//for asking for both states (liquid and gas) calculation
    char state;//we will recive here the state of the calculation
    double MW;
    double liqFraction;
    FF_SaftEOSdata dataP;
    FF_SWEOSdata  dataS;
    FF_CubicEOSdata dataC;
    char var;

    //First we need to get the eos data
    if (subs->eosType==FF_SAFTtype) MW=subs->saftData.MW;
    else if (subs->eosType==FF_SWtype) MW=subs->swData.MW;
    else MW=subs->cubicData.MW;


    //Later we charge the known variables
    switch (ui->cbSubsCalcKnownVars->currentIndex()){
    case 0://P,H
        var='H';
        thR.P=ui->leSubsCalcP->text().toDouble()*1e5;
        thR.H=MW*ui->leSubsCalcH->text().toDouble();
        break;
    case 1://P,U
        var='U';
        thR.P=ui->leSubsCalcP->text().toDouble()*1e5;
        thR.U=MW*ui->leSubsCalcU->text().toDouble();
        break;
    case 2://P,S
        var='S';
        thR.P=ui->leSubsCalcP->text().toDouble()*1e5;
        thR.S=MW*ui->leSubsCalcS->text().toDouble();
        break;
    case 3://Rho,T
        var='T';
        thR.V=MW/ui->leSubsCalcRho->text().toDouble()*1e-3;
        thR.T=ui->leSubsCalcT->text().toDouble()+273.15;
        break;
    case 4://Rho,H
        var='H';
        thR.V=MW/ui->leSubsCalcRho->text().toDouble()*1e-3;
        thR.H=MW*ui->leSubsCalcH->text().toDouble();
        break;
    }

    //Now we make calculations
    switch (ui->cbSubsCalcKnownVars->currentIndex()){
    case 0://P,H
    case 1://P,U
    case 2://P,S
        if (subs->eosType==FF_SAFTtype) FF_ThermoEOSfromPX(&subs->eosType,&subs->saftData,&subs->cp0Corr.form,subs->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else if (subs->eosType==FF_SWtype) FF_ThermoEOSfromPX(&subs->eosType,&subs->swData,&subs->cp0Corr.form,subs->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else FF_ThermoEOSfromPX(&subs->eosType,&subs->cubicData,&subs->cp0Corr.form,subs->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        break;
    case 3:
    case 4:
        if (subs->eosType==FF_SAFTtype) FF_ThermoEOSfromVX(&subs->eosType,&subs->saftData,&subs->cp0Corr.form,subs->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else if (subs->eosType==FF_SWtype) FF_ThermoEOSfromVX(&subs->eosType,&subs->swData,&subs->cp0Corr.form,subs->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else FF_ThermoEOSfromVX(&subs->eosType,&subs->cubicData,&subs->cp0Corr.form,subs->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        break;
    }

    //And finally we write the data
    ui->leSubsCalcT->setText(QString::number(thR.T-273.15));
    ui->leSubsCalcRho->setText(QString::number(MW/thR.V*1e-3));
    ui->leSubsCalcP->setText(QString::number(thR.P*1e-5));
    ui->leSubsCalcH->setText(QString::number(thR.H/MW));
    ui->leSubsCalcU->setText(QString::number(thR.U/MW));
    ui->leSubsCalcS->setText(QString::number(thR.S/MW));
    ui->leSubsCalcLiqFrac->setText(QString::number(liqFraction));
}

//Slot for table content exportation in csv ; delimited format
void FreeFluidsMainWindow::twSubsCalcExport()
{
    QFileDialog *dia = new QFileDialog(this,"Choose directory and file name");
    dia->setNameFilter("*.csv *.txt");
    dia->showNormal();
    QString fileName;
    if (dia->exec())
        fileName = dia->selectedFiles().first();
    std::cout<<fileName.toStdString()<<std::endl;
    QFile *file=new QFile(fileName,this);
    if (file->open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(file);
        for (int i=0;i<ui->twSubsCalc->rowCount();i++){
            out<< ui->twSubsCalc->verticalHeaderItem(i)->text().toUtf8()<<";";
            for (int j=0;j<ui->twSubsCalc->columnCount();j++){
                out << ui->twSubsCalc->item(i,j)->text().toUtf8() << ";";
            }
            out << "\n";
        }
    }
    file->close();
    delete dia;
    delete file;
}

//Slot for substance exportation in binary format
void FreeFluidsMainWindow::btnSubsCalcExportSubs(){
    QFileDialog *dia = new QFileDialog(this,"Choose directory and file name");
    dia->showNormal();
    QString fileName;
    if (dia->exec())
        fileName = dia->selectedFiles().first();

    FILE *outfile;
    // open file for writing
    outfile = fopen (fileName.toStdString().c_str(),"wb");
    if (outfile == NULL)
    {
        fprintf(stderr, "\nError opend file\n");
        delete dia;
        exit (1);
    }
    if ((subsData->cubicData.eos==0)&&(subsData->baseProp.MW>0)&&(subsData->baseProp.Tc>0)&&(subsData->baseProp.Pc>0)&&(subsData->baseProp.w>0)){
        subsData->cubicData.eos=FF_PR78;
        subsData->cubicData.MW=subsData->baseProp.MW;
        subsData->cubicData.Tc=subsData->baseProp.Tc;
        subsData->cubicData.Pc=subsData->baseProp.Pc;
        subsData->cubicData.w=subsData->baseProp.w;
        subsData->cubicData.c=0;
    }
    fwrite (subsData, sizeof(FF_SubstanceData), 1, outfile);
    fclose(outfile);
    delete dia;
}

//Slot for transfer from eos calculation to correlation calculation data tables
void FreeFluidsMainWindow::btnSubsCalcTransfer(){
    int i,n=3;
    for (i=0;i<11;i++)ui->twSubsTools->item(i,0)->setText(QString::number(ui->twSubsCalc->item(0,i)->text().toDouble()+273.15));//We fill the temperatures
    if (ui->chbSubsCalcTransfVp->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(20,i)->text().toDouble()*1e5));
    n++;
    }
    if (ui->chbSubsCalcTransfSLd->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(ui->twSubsCalc->item(21,i)->text());
    n++;
    }
    if (ui->chbSubsCalcTransfSGd->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(ui->twSubsCalc->item(22,i)->text());
    n++;
    }
    if (ui->chbSubsCalcTransfHv->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(23,i)->text().toDouble()*1e3));
    n++;
    }
}

//Substance tools tab functions
//*****************************

//Slot for correlation selection load on substance and screen
void FreeFluidsMainWindow::cbSubsToolsCorrLoad(int position){
    QString corrProp;
    FF_Correlation corr;
    corrProp=subsToolsCorrModel->record(position).value("Property").toString();
    corr.id=subsToolsCorrModel->record(ui->cbSubsToolsSelCorr->currentIndex()).value("Id").toInt();
    GetCorrDataById2(&corr,&db);

//****************************
//============================

    if (corrProp=="Cp0"){
        subs->cp0Corr=corr;
        subsData->cp0Corr=corr;
        ui->leSubsToolsCp0Corr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        //std::cout<<subs->lDensCorr.coef[0]<<std::endl;
        ui->chbSubsToolsCp0->setChecked(true);
        ui->leSubsCalcCp0->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        ui->chbSubsCalcCp0->setChecked(true);
    }

    else if (corrProp=="Ldens"){
        subs->lDensCorr=corr;
        subsData->lDensCorr=corr;
        ui->leSubsToolsLdensCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        //std::cout<<subs->lDensCorr.coef[0]<<std::endl;
        ui->chbSubsToolsLdens->setChecked(true);
    }
    else if (corrProp=="Vp"){
        subs->vpCorr=corr;
        subsData->vpCorr=corr;
        ui->leSubsToolsVpCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        ui->chbSubsToolsVp->setChecked(true);
    }
    else if (corrProp=="Bt"){
        //subs->BtCorr=corr;
        subsData->btCorr=corr;
        ui->leSubsToolsBtCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        ui->chbSubsToolsBt->setChecked(true);
    }
    else if (corrProp=="HvSat"){
            subs->hVsatCorr=corr;
            subsData->hVsatCorr=corr;
            ui->leSubsToolsHvCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsHv->setChecked(true);
    }
    else if (corrProp=="LCp"){
            subs->lCpCorr=corr;
            subsData->lCpCorr=corr;
            ui->leSubsToolsLCpCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLCp->setChecked(true);
    }
    else if (corrProp=="Lvisc"){
            subs->lViscCorr=corr;
            subsData->lViscCorr=corr;
            ui->leSubsToolsLviscCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLvisc->setChecked(true);
    }
    else if (corrProp=="LthC"){
            subs->lThCCorr=corr;
            subsData->lThCCorr=corr;
            ui->leSubsToolsLthCCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLthC->setChecked(true);
    }
    else if (corrProp=="LsurfT"){
            subs->lSurfTCorr=corr;
            subsData->lSurfTCorr=corr;
            ui->leSubsToolsLsurfTCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLsurfT->setChecked(true);
    }
    else if (corrProp=="GdensSat"){
            subs->gDensCorr=corr;
            subsData->gDensCorr=corr;
            ui->leSubsToolsGdensCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsGdens->setChecked(true);
    }
    else if (corrProp=="Gvisc"){
            subs->gViscCorr=corr;
            subsData->gViscCorr=corr;
            ui->leSubsToolsGviscCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsGvisc->setChecked(true);
    }
    else if (corrProp=="GthC"){
            subs->gThCCorr=corr;
            subsData->gThCCorr=corr;
            ui->leSubsToolsGthCCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsGthC->setChecked(true);
    }
    else if (corrProp=="Sdens"){
            subs->sDensCorr=corr;
            subsData->sDensCorr=corr;
            ui->leSubsToolsSdensCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsSdens->setChecked(true);
    }
    else if (corrProp=="SCp"){
            subs->sCpCorr=corr;
            subsData->sCpCorr=corr;
            ui->leSubsToolsSCpCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsSCp->setChecked(true);
    }
    //printf("Lvisc A:%f\n",subsData->lViscCorr.coef[0]);
}

//Slot for filling table with data from correlations
void FreeFluidsMainWindow::btnSubsToolsFillTable(){
    int n=1;//The table column where we will write next data from correlation calculation
    int i;//the table row index

    //Now we make calculations
    double minT=ui->leSubsToolsMinTemp->text().toDouble()+273.15;
    double maxT=ui->leSubsToolsMaxTemp->text().toDouble()+273.15;
    int nPoints=ui->leSubsToolsNumPoints->text().toInt();
    double T[nPoints],y[nPoints];
    double Tincrement=(maxT-minT)/(nPoints-1);//temperature increments between calculations
    for (i=0;i<nPoints;i++) T[i]=minT+i*Tincrement;
    int initRow=ui->leSubsToolsInitRow->text().toInt();
    for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,0)->setText(QString::number(T[i-initRow+1]));//We fill the temperatures
    if (ui->chbSubsToolsCp0->isChecked()==true){
        FF_PhysPropCorr(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Cp0");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the vapor pressure
    n++;
    }
    if (ui->chbSubsToolsVp->isChecked()==true){
        FF_PhysPropCorr(&subs->vpCorr.form,subs->vpCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Vapor press.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the vapor pressure
    n++;
    }
    /*if (ui->chbSubsToolsBt->isChecked()==true){
        FF_PhysPropCorr(&subs->btCorr.form,subs->btCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Boil T.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the boiling temperature
    n++;
    }*/
    if (ui->chbSubsToolsLdens->isChecked()==true){
        FF_PhysPropCorr(&subs->lDensCorr.form,subs->lDensCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);//we call the calculation routine
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.dens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the liquid density
    n++;
    }
    if (ui->chbSubsToolsHv->isChecked()==true){
        FF_PhysPropCorr(&subs->hVsatCorr.form,subs->hVsatCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Hv");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the heat of vaporization
    n++;
    }
    if (ui->chbSubsToolsLCp->isChecked()==true){
        FF_PhysPropCorr(&subs->lCpCorr.form,subs->lCpCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.Cp");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the liquid Cp
    n++;
    }
    if (ui->chbSubsToolsLvisc->isChecked()==true){
        FF_PhysPropCorr(&subs->lViscCorr.form,subs->lViscCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.visc.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the liquid viscosity
    n++;
    }
    if ((ui->chbSubsToolsLthC->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subs->lThCCorr.form,subs->lThCCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.th.cond.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the liquid thermal conductivity
    n++;
    }
    if ((ui->chbSubsToolsLsurfT->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subs->lSurfTCorr.form,subs->lSurfTCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.s.tens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the surface tension
    n++;
    }
    if ((ui->chbSubsToolsGdens->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subs->gDensCorr.form,subs->gDensCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Gas dens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the gas saturated density
    n++;
    }
    if ((ui->chbSubsToolsGvisc->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subs->gViscCorr.form,subs->gViscCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Gas visc.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the gas viscosity
    n++;
    }
    if ((ui->chbSubsToolsGthC->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subs->gThCCorr.form,subs->gThCCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Gas th.con.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the gas thermal conductivity
    n++;
    }
    if ((ui->chbSubsToolsSdens->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subs->sDensCorr.form,subs->sDensCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Sol.dens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the gas thermal conductivity
    n++;
    }
    if ((ui->chbSubsToolsSCp->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subs->sCpCorr.form,subs->sCpCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Sol.Cp");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the gas thermal conductivity
    n++;
    }
    if ((ui->chbSubsToolsVpAmbrose->isChecked()==true)&&(n<6)){
        //std::cout<<T[0]<<std::endl;
        FF_VpAmbroseWalton(&subs->baseProp,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Vp Ambrose");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the Ambrose vapor pressure
    n++;
    }
    if ((ui->chbSubsToolsVpRiedel->isChecked()==true)&&(n<6)){
        double tRef,pRef;
        int type;
        if (ui->rbSubsToolsRiedelAl->isChecked())type=1;
        else if (ui->rbSubsToolsRiedelAc->isChecked())type=2;
        else type=0;
        tRef=ui->leSubsToolsVpTref->text().toDouble();
        pRef=ui->leSubsToolsVpPref->text().toDouble();
        //std::cout<<T[0]<<std::endl;
        FF_VpRiedelVetere(&subsData->baseProp.Tc,&subsData->baseProp.Pc,&tRef,&pRef,&type,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Vp Riedel");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the Ambrose vapor pressure
    n++;
    }
    if ((ui->chbSubsToolsLdensRa->isChecked()==true)&&(n<6)){
        double tRef,dRef;
        tRef=ui->leSubsToolsLdensTref->text().toDouble();
        dRef=ui->leSubsToolsLdensDref->text().toDouble();
        //std::cout<<T[0]<<std::endl;
        FF_LiqDensSatRackett(&subs->baseProp,&tRef,&dRef,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.d.Rack.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the Rackett density
    n++;
    }
}

//Slot for table content importation from csv ; delimited format
void FreeFluidsMainWindow::twSubsToolsImport()
{
    QFileDialog *dia = new QFileDialog(this,"Choose directory and file name");
    dia->setNameFilter("*.csv *.txt");
    dia->showNormal();
    QString fileName;
    QString line;
    int i,j;
    int position;
    if (dia->exec())
        fileName = dia->selectedFiles().first();
    std::cout<<fileName.toStdString()<<std::endl;
    QFile *file=new QFile(fileName,this);
    if (file->open(QFile::ReadOnly | QIODevice::Text)) {
        QTextStream in(file);
        i=ui->leSubsToolsInitRow->text().toInt()-1;
        j=ui->leSubsToolsColumnNum->text().toInt()-1;
        if ((j<=0)||(j>=ui->twSubsTools->columnCount())) j=ui->twSubsTools->columnCount()-1;//we acotate the selected column
        while (!in.atEnd()) {
                line = in.readLine();
                position=line.indexOf(";",0);
                ui->twSubsTools->item(i,0)->setText(line.left(position));
                ui->twSubsTools->item(i,j)->setText(line.right(line.length()-position-1));
                i++;
            }
    }

    file->close();
    delete dia;
    delete file;
}

//Slot for fixing the bounds of the needed parameters
void FreeFluidsMainWindow::cbSubsToolsSelOptCorr(int position){
    int i,j;//the loop variables
    for (i=0;i<ui->twSubsToolsCoefPrep->rowCount();i++) for (j=0;j<ui->twSubsToolsCoefPrep->columnCount();j++) ui->twSubsToolsCoefPrep->item(i,j)->setText("");//we clear the content
    switch (position){
    case FF_DIPPR106:
    case FF_DIPPR106Hv:
    case FF_DIPPR106Ld:
    case FF_DIPPR106SurfT:
        ui->twSubsToolsCoefPrep->item(0,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(1,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(2,5)->setText(ui->leSubsToolsTc->text());
        break;
    case FF_PCWIN:
        ui->twSubsToolsCoefPrep->item(0,4)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(1,4)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(2,4)->setText(ui->leSubsToolsTc->text());
        break;
    case FF_DIPPR116:
    case FF_DIPPR116Ld:
    case FF_PPDS10:
        ui->twSubsToolsCoefPrep->item(0,0)->setText(ui->leSubsToolsRc ->text());
        ui->twSubsToolsCoefPrep->item(1,0)->setText(ui->leSubsToolsRc->text());
        ui->twSubsToolsCoefPrep->item(2,0)->setText(ui->leSubsToolsRc->text());
        ui->twSubsToolsCoefPrep->item(0,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(1,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(2,5)->setText(ui->leSubsToolsTc->text());
        break;
    case FF_Wagner25:
    case FF_Wagner36:
        ui->twSubsToolsCoefPrep->item(0,0)->setText(ui->leSubsToolsPc->text());
        ui->twSubsToolsCoefPrep->item(1,0)->setText(ui->leSubsToolsPc->text());
        ui->twSubsToolsCoefPrep->item(2,0)->setText(ui->leSubsToolsPc->text());
        ui->twSubsToolsCoefPrep->item(0,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(1,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(2,5)->setText(ui->leSubsToolsTc->text());
        break;
    case FF_WagnerGd:
        ui->twSubsToolsCoefPrep->item(0,0)->setText(ui->leSubsToolsRc->text());
        ui->twSubsToolsCoefPrep->item(1,0)->setText(ui->leSubsToolsRc->text());
        ui->twSubsToolsCoefPrep->item(2,0)->setText(ui->leSubsToolsRc->text());
        ui->twSubsToolsCoefPrep->item(0,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(1,5)->setText(ui->leSubsToolsTc->text());
        ui->twSubsToolsCoefPrep->item(2,5)->setText(ui->leSubsToolsTc->text());
        break;

    }
}

//Slot for clearing content of the forced coeffients table, and optimization errors
void FreeFluidsMainWindow::btnSubsToolsClearCoef(){
    int i,j;//the loop variables
    for (i=0;i<ui->twSubsToolsCoefPrep->rowCount();i++) for (j=0;j<ui->twSubsToolsCoefPrep->columnCount();j++) ui->twSubsToolsCoefPrep->item(i,j)->setText("");//we clear the content
}

//Slot for correlation coefficients calculation
void FreeFluidsMainWindow::btnSubsToolsFindCorr(){
    FF_CorrelationData data;
    int fromRow,toRow,cor,numCoef,i;
    //We clear the results
    ui->leSubsToolsCoef0->setText("");
    ui->leSubsToolsCoef1->setText("");
    ui->leSubsToolsCoef2->setText("");
    ui->leSubsToolsCoef3->setText("");
    ui->leSubsToolsCoef4->setText("");
    ui->leSubsToolsCoef5->setText("");

    //We need to know where is the data, the correlation to use and the number of coefficients to find
    fromRow=ui->leSubsToolsFromRow->text().toInt();
    toRow=ui->leSubsToolsToRow->text().toInt();
    data.nPoints=toRow-fromRow+1;
    cor=ui->cbSubsToolsSelOptCorr->currentIndex();
    data.eq=(FF_CorrEquation)cor;
    for (i=0;i<data.nPoints;i++){//We read the data in the table
        data.x[i]=ui->twSubsTools->item(i+fromRow-1,0)->text().toDouble();
        data.y[i]=ui->twSubsTools->item(i+fromRow-1,1)->text().toDouble();
    }
    switch(data.eq){
    case FF_Polynomial:
    //case FF_Polynomial2:
    case FF_DIPPR106:
    case FF_DIPPR106Hv:
    case FF_DIPPR106Ld:
    case FF_DIPPR106SurfT:
    case FF_DIPPR116:
    case FF_DIPPR116Ld:
    case FF_PPDS10:
    case FF_Wagner25:
    case FF_Wagner36:
    case FF_WagnerGd:
        numCoef=6;
        break;
    case FF_Rackett:
        numCoef=4;
        break;
    case FF_Antoine1:
    case FF_Antoine2:
        numCoef=3;
        break;
    default:
        numCoef=5;
        break;
    }

    double lb[numCoef],ub[numCoef],coef[numCoef],error;
    char enforce[numCoef];
    for (i=0;i<numCoef;i++){//We read the limits to coefficients, if established
        if ((ui->twSubsToolsCoefPrep->item(0,i)->text().toStdString()>" ")&&(ui->twSubsToolsCoefPrep->item(1,i)->text().toStdString()>" ")&&
           (ui->twSubsToolsCoefPrep->item(2,i)->text().toStdString()>" ")){
           lb[i]=ui->twSubsToolsCoefPrep->item(0,i)->text().toDouble();
           ub[i]=ui->twSubsToolsCoefPrep->item(1,i)->text().toDouble();
           coef[i]=ui->twSubsToolsCoefPrep->item(2,i)->text().toDouble();
           enforce[i]='y';
        }
    }
    FF_OptCorrelation(numCoef,lb,ub,enforce,&data,coef,&error);
    //if (cor==7) ui->leSubsToolsCoef0->setText(ui->leSubsToolsRc->text());
    //else if (cor==8) ui->leSubsToolsCoef0->setText(ui->leSubsToolsPc->text());
    ui->leSubsToolsCoef0->setText(QString::number(coef[0]));
    ui->leSubsToolsCoef1->setText(QString::number(coef[1]));
    ui->leSubsToolsCoef2->setText(QString::number(coef[2]));
    if (numCoef>3) ui->leSubsToolsCoef3->setText(QString::number(coef[3]));
    if (numCoef>4) ui->leSubsToolsCoef4->setText(QString::number(coef[4]));
    if (numCoef>5) ui->leSubsToolsCoef5->setText(QString::number(coef[5]));
    ui->leSubsToolsCorrError->setText(QString::number(error*100));
}

//Slot for EOS coefficients calculation
void FreeFluidsMainWindow::btnSubsToolsFindEOS(){
    unsigned optTime;
    if (ui->spbSubsToolsTime->value()==0) optTime=30;
    else optTime=60*ui->spbSubsToolsTime->value();
    FF_EOSPvRhoData data;
    //We need to know where is the data, the EOS to use
    int fromRow,toRow,eos,numCoef,i,j;
    fromRow=ui->leSubsToolsFromRow->text().toInt();
    toRow=ui->leSubsToolsToRow->text().toInt();
    data.nPoints=toRow-fromRow+1;
    data.MW=ui->leSubsToolsMW->text().toDouble();
    data.Tc=ui->leSubsToolsTc->text().toDouble();
    data.Pc=ui->leSubsToolsPc->text().toDouble();
    data.Zc=ui->leSubsToolsZc->text().toDouble();
    data.w=ui->leSubsToolsW->text().toDouble();
    data.VdWV=ui->leSubsToolsVdWV->text().toDouble();
    data.ldensFilter=ui->spbSubsToolsDens->value() /100.0;
    data.zcFilter=ui->spbSubsToolsZc->value()/100.0;
    printf("%f %f\n",data.ldensFilter,data.zcFilter);
    //We clear the results
    ui->leSubsToolsCoef0->setText("");
    ui->leSubsToolsCoef1->setText("");
    ui->leSubsToolsCoef2->setText("");
    ui->leSubsToolsCoef3->setText("");
    ui->leSubsToolsCoef4->setText("");
    //We assign the EOS to use, and the number of coefficients to find
    //printf("Hola, buscando\n");
    eos=ui->cbSubsToolsSelOptEOS->currentIndex();
    numCoef=3;
    switch (eos){
    case 1:
        data.eos=FF_PR78;
        break;
    case 2:
        data.eos=FF_PRSV1;
        numCoef=1;
        break;
    case 3:
        data.eos=FF_PRMELHEM;
        numCoef=2;
        break;
    case 4:
        data.eos=FF_PRALMEIDA;
        break;
    case 5:
        data.eos=FF_PRSOF;
        numCoef=2;
        break;
    case 6:
        data.eos=FF_PRMC;
        break;
    case 7:
        data.eos=FF_PRTWU91;
        break;
    case 8:
        data.eos=FF_PRFIT4;
        numCoef=4;
        break;
    case 9:
        data.eos=FF_PRvTWU91;
        break;
    case 10:
        data.eos=FF_SRKSOF;
        numCoef=2;
        break;
    case 11:
        data.eos=FF_SRKMC;
        break;
    case 12:
        data.eos=FF_SRKTWU91;
        break;
    case 13:
        data.eos=FF_PCSAFT;
        break;
    case 14:
        data.eos=FF_PCSAFT2B;
        numCoef=5;
        break;
    case 15:
        data.eos=FF_PCSAFT4C;
        numCoef=5;
        break;
    case 16:
        data.eos=FF_PCSAFT1;
        numCoef=5;
        break;
    }
    for (i=0;i<data.nPoints;i++){//We read the data in the table
        data.points[i][0]=ui->twSubsTools->item(i+fromRow-1,0)->text().toDouble();
        data.points[i][1]=ui->twSubsTools->item(i+fromRow-1,1)->text().toDouble();
        data.points[i][2]=ui->twSubsTools->item(i+fromRow-1,2)->text().toDouble();
        //printf("%f %f %f\n",data.points[i][0],data.points[i][1],data.points[i][2]);
    }
    double lb[numCoef],ub[numCoef],coef[numCoef],error;
    char enforce[numCoef];
    for (i=0;i<numCoef;i++){//We read the limits to coefficients, if established
        if ((ui->twSubsToolsCoefPrep->item(0,i)->text().toStdString()>" ")&&(ui->twSubsToolsCoefPrep->item(1,i)->text().toStdString()>" ")&&
           (ui->twSubsToolsCoefPrep->item(2,i)->text().toStdString()>" ")){
           lb[i]=ui->twSubsToolsCoefPrep->item(0,i)->text().toDouble();
           ub[i]=ui->twSubsToolsCoefPrep->item(1,i)->text().toDouble();
           coef[i]=ui->twSubsToolsCoefPrep->item(2,i)->text().toDouble();
           enforce[i]='y';
        }
    }
    FF_OptEOSparam(optTime,numCoef,lb,ub,enforce,&data,coef,&error);//This is the calculation
    //And now we write the results
    ui->leSubsToolsCoef0->setText(QString::number(coef[0]));
    if (numCoef>1) ui->leSubsToolsCoef1->setText(QString::number(coef[1]));
    if (numCoef>2) ui->leSubsToolsCoef2->setText(QString::number(coef[2]));
    if (numCoef>3) ui->leSubsToolsCoef3->setText(QString::number(coef[3]));
    if (numCoef>4) ui->leSubsToolsCoef4->setText(QString::number(coef[4]));
    ui->leSubsToolsCorrError->setText(QString::number(error*100));
    ui->leSubsToolsVpError->setText(QString::number(data.vpError*100));
    ui->leSubsToolsLdensError->setText(QString::number(data.ldensError*100));
    ui->leSubsToolsZcError->setText(QString::number(data.zcError*100));
}

//slot for EOS coefficients test
void FreeFluidsMainWindow::btnSubsToolsTestEOS(){

    //We need to know where is the data, the EOS to use
    int eos;

    subsData->baseProp.MW=ui->leSubsToolsMW->text().toDouble();
    subsData->baseProp.Tc=ui->leSubsToolsTc->text().toDouble();
    subsData->baseProp.Pc=ui->leSubsToolsPc->text().toDouble();
    subsData->baseProp.Zc=ui->leSubsToolsZc->text().toDouble();
    subsData->baseProp.w=ui->leSubsToolsW->text().toDouble();

    //We assign the EOS to use, and the number of coefficients to find
    //printf("Hola, buscando\n");
    eos=ui->cbSubsToolsSelOptEOS->currentIndex();
    if (eos<13){
        subs->eosType=FF_CubicType;
        ui->leSubsCalcCubic->setText("From optimization coefficients");
        ui->chbSubsCalcCubic->setChecked(true);
        subsData->cubicData.MW=ui->leSubsToolsMW->text().toDouble();
        subsData->cubicData.Tc=ui->leSubsToolsTc->text().toDouble();
        subsData->cubicData.Pc=ui->leSubsToolsPc->text().toDouble();
        subsData->cubicData.Zc=ui->leSubsToolsZc->text().toDouble();
        subsData->cubicData.w=ui->leSubsToolsW->text().toDouble();
        subsData->cubicData.VdWV=ui->leSubsToolsVdWV->text().toDouble();
        subsData->cubicData.c=0.0;
    }
    else{
        subs->eosType=FF_SAFTtype;
        ui->leSubsCalcSaft->setText("From optimization coefficients");
        ui->chbSubsCalcSaft->setChecked(true);
        subsData->saftData.MW=ui->leSubsToolsMW->text().toDouble();
        subsData->saftData.Tc=ui->leSubsToolsTc->text().toDouble();
        subsData->saftData.Pc=ui->leSubsToolsPc->text().toDouble();
        subsData->saftData.Zc=ui->leSubsToolsZc->text().toDouble();
        subsData->saftData.w=ui->leSubsToolsW->text().toDouble();
        subsData->saftData.sigma=ui->leSubsToolsCoef0->text().toDouble();
        subsData->saftData.m=ui->leSubsToolsCoef1->text().toDouble();
        subsData->saftData.epsilon=ui->leSubsToolsCoef2->text().toDouble();
        subsData->saftData.mu=0.0;
        subsData->saftData.xp=0.0;
    }

    switch (eos){
    case 1:
        subsData->cubicData.eos=FF_PR78;
        subsData->cubicData.Tc=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.Pc=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.w=ui->leSubsToolsCoef2->text().toDouble();
        break;
    case 2:
        subsData->cubicData.eos=FF_PRSV1;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        break;
    case 3:
        subsData->cubicData.eos=FF_PRMELHEM;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        break;
    case 4:
        subsData->cubicData.eos=FF_PRALMEIDA;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
        break;
    case 5:
        subsData->cubicData.eos=FF_PRSOF;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        break;
    case 6:
        subsData->cubicData.eos=FF_PRMC;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
        break;
    case 7:
        subsData->cubicData.eos=FF_PRTWU91;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
        break;
    case 8:
        subsData->cubicData.eos=FF_PRFIT4;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
        subsData->cubicData.k4=ui->leSubsToolsCoef3->text().toDouble();
        subsData->cubicData.c=-1.0;
        break;
    case 9:
        subsData->cubicData.eos=FF_PRvTWU91;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
        break;
    case 10:
        subsData->cubicData.eos=FF_SRKSOF;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        break;
    case 11:
        subsData->cubicData.eos=FF_SRKMC;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
        break;
    case 12:
        subsData->cubicData.eos=FF_SRKTWU91;
        subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
        subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
        subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
    case 13:
        subsData->saftData.eos=FF_PCSAFT;
        subsData->saftData.nAcid=0;
        subsData->saftData.nPos=0;
        subsData->saftData.nNeg=0;
        break;
    case 14:
        subsData->saftData.eos=FF_PCSAFT2B;
        subsData->saftData.kAB=ui->leSubsToolsCoef3->text().toDouble();
        subsData->saftData.epsilonAB=ui->leSubsToolsCoef4->text().toDouble();
        subsData->saftData.nAcid=0;
        subsData->saftData.nPos=1;
        subsData->saftData.nNeg=1;
        break;
    case 15:
        subsData->saftData.eos=FF_PCSAFT4C;
        subsData->saftData.kAB=ui->leSubsToolsCoef3->text().toDouble();
        subsData->saftData.epsilonAB=ui->leSubsToolsCoef4->text().toDouble();
        subsData->saftData.nAcid=0;
        subsData->saftData.nPos=2;
        subsData->saftData.nNeg=2;
        break;
    case 16:
        subsData->saftData.eos=FF_PCSAFT1;
        subsData->saftData.kAB=ui->leSubsToolsCoef3->text().toDouble();
        subsData->saftData.epsilonAB=ui->leSubsToolsCoef4->text().toDouble();
        subsData->saftData.nAcid=1;
        subsData->saftData.nPos=0;
        subsData->saftData.nNeg=0;
        break;
    }
}

//Slot for clearing content in the table
void  FreeFluidsMainWindow::twSubsToolsClear(){
    int i,j;//the loop variables
    for (i=0;i<ui->twSubsTools->rowCount();i++) for (j=0;j<ui->twSubsTools->columnCount();j++) ui->twSubsTools->item(i,j)->setText("");//we clear the content
}

//Slot for interchanging table columns
void FreeFluidsMainWindow::twSubsToolsInterchange(){
    int a,b;
    QString data;
    a = ui->leSubsToolsColA->text().toInt()-1;
    b = ui->leSubsToolsColB->text().toInt()-1;
    data=ui->twSubsTools->horizontalHeaderItem(a)->text();
    ui->twSubsTools->horizontalHeaderItem(a)->setText( ui->twSubsTools->horizontalHeaderItem(b)->text());
    ui->twSubsTools->horizontalHeaderItem(b)->setText(data);
    for (int i=0;i<ui->twSubsTools->rowCount();i++){
        data=ui->twSubsTools->item(i,a)->text();
        ui->twSubsTools->item(i,a)->setText(ui->twSubsTools->item(i,b)->text());
        ui->twSubsTools->item(i,b)->setText(data);
    }

}

//Slot for do operation on a data column
void FreeFluidsMainWindow::twSubsToolsDoOperation(){
    double aux;
    if (ui->rbSubsToolsAdd->isChecked()){
        for (int i=0;i<ui->twSubsTools->rowCount();i++){
            aux=ui->twSubsTools->item(i,ui->spbSubsToolsCol->value()-1)->text().toDouble();
            ui->twSubsTools->item(i,ui->spbSubsToolsCol->value()-1)->setText(QString::number(aux+ui->leSubsToolsOp->text().toDouble()));
        }
    }
    else if (ui->rbSubsToolsMultiply->isChecked()){
        for (int i=0;i<ui->twSubsTools->rowCount();i++){
            aux=ui->twSubsTools->item(i,ui->spbSubsToolsCol->value()-1)->text().toDouble();
            ui->twSubsTools->item(i,ui->spbSubsToolsCol->value()-1)->setText(QString::number(aux*ui->leSubsToolsOp->text().toDouble()));
        }
    }

}

//Slot for adding EOS to database
void FreeFluidsMainWindow::btnSubsToolAddEos(){
    double lowLimit,highLimit;
    QString comment;
    lowLimit=ui->leSubsToolsAddLowT->text().toDouble();
    highLimit=ui->leSubsToolsAddHighT->text().toDouble();
    comment=ui->leSubsToolsAddComment->text();
    int eos;
    enum FF_EosType eosType;
    //We assign the EOS to use, and the number of coefficients to find
    eos=ui->cbSubsToolsSelOptEOS->currentIndex();
    if (eos<13){
        eosType=FF_CubicType;
        FF_CubicEOSdata cubicData;
        cubicData.MW=ui->leSubsToolsMW->text().toDouble();
        cubicData.Tc=ui->leSubsToolsTc->text().toDouble();
        cubicData.Pc=ui->leSubsToolsPc->text().toDouble();
        cubicData.Zc=ui->leSubsToolsZc->text().toDouble();
        cubicData.w=ui->leSubsToolsW->text().toDouble();
        cubicData.VdWV=ui->leSubsToolsVdWV->text().toDouble();
        cubicData.c=0.0;
        switch (eos){
        case 1:
            cubicData.eos=FF_PR78;
            cubicData.Tc=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.Pc=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.w=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case 2:
            cubicData.eos=FF_PRSV1;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            break;
        case 3:
            cubicData.eos=FF_PRMELHEM;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            break;
        case 4:
            cubicData.eos=FF_PRALMEIDA;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case 5:
            cubicData.eos=FF_PRSOF;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            break;
        case 6:
            cubicData.eos=FF_PRMC;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case 7:
            cubicData.eos=FF_PRTWU91;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case 8:
            cubicData.eos=FF_PRFIT4;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            cubicData.k4=ui->leSubsToolsCoef3->text().toDouble();
            cubicData.c=-1.0;
            break;
        case 9:
            cubicData.eos=FF_PRvTWU91;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case 10:
            cubicData.eos=FF_SRKSOF;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            break;
        case 11:
            cubicData.eos=FF_SRKMC;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case 12:
            cubicData.eos=FF_SRKTWU91;
            cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            break;
        }
        AddEosToDataBase(subs->id,eosType,&cubicData,&lowLimit,&highLimit,&comment,&db);
    }
    else{
        eosType=FF_SAFTtype;
        FF_SaftEOSdata saftData;
        saftData.MW=ui->leSubsToolsMW->text().toDouble();
        saftData.Tc=ui->leSubsToolsTc->text().toDouble();
        saftData.Pc=ui->leSubsToolsPc->text().toDouble();
        saftData.Zc=ui->leSubsToolsZc->text().toDouble();
        saftData.w=ui->leSubsToolsW->text().toDouble();
        saftData.sigma=ui->leSubsToolsCoef0->text().toDouble();
        saftData.m=ui->leSubsToolsCoef1->text().toDouble();
        saftData.epsilon=ui->leSubsToolsCoef2->text().toDouble();
        saftData.mu=0.0;
        saftData.xp=0.0;
        switch(eos){
        case 13:
            saftData.eos=FF_PCSAFT;
            saftData.kAB=0;
            saftData.epsilonAB=0;
            saftData.nAcid=0;
            saftData.nPos=0;
            saftData.nNeg=0;
            break;
        case 14:
            saftData.eos=FF_PCSAFT2B;
            saftData.kAB=ui->leSubsToolsCoef3->text().toDouble();
            saftData.epsilonAB=ui->leSubsToolsCoef4->text().toDouble();
            saftData.nAcid=0;
            saftData.nPos=1;
            saftData.nNeg=1;
            break;
        case 15:
            saftData.eos=FF_PCSAFT4C;
            saftData.kAB=ui->leSubsToolsCoef3->text().toDouble();
            saftData.epsilonAB=ui->leSubsToolsCoef4->text().toDouble();
            saftData.nAcid=0;
            saftData.nPos=2;
            saftData.nNeg=2;
            break;
        case 16:
            saftData.eos=FF_PCSAFT1;
            saftData.kAB=ui->leSubsToolsCoef3->text().toDouble();
            saftData.epsilonAB=ui->leSubsToolsCoef4->text().toDouble();
            saftData.nAcid=1;
            saftData.nPos=0;
            saftData.nNeg=0;
            break;
        }
        //printf("saftData.eos:%i\n",saftData.eos);
        AddEosToDataBase(subs->id,eosType,&saftData,&lowLimit,&highLimit,&comment,&db);
    }

}

//Slot for adding correlation to database
void FreeFluidsMainWindow::btnSubsToolAddCorr(){
    double lowLimit,highLimit;
    QString comment;
    int selection;
    FF_Correlation corr;
    lowLimit=ui->leSubsToolsAddLowT->text().toDouble();
    highLimit=ui->leSubsToolsAddHighT->text().toDouble();
    comment=ui->leSubsToolsAddComment->text();
    corr.coef[0]=ui->leSubsToolsCoef0->text().toDouble();
    corr.coef[1]=ui->leSubsToolsCoef1->text().toDouble();
    corr.coef[2]=ui->leSubsToolsCoef2->text().toDouble();
    corr.coef[3]=ui->leSubsToolsCoef3->text().toDouble();
    corr.coef[4]=ui->leSubsToolsCoef4->text().toDouble();
    corr.coef[5]=ui->leSubsToolsCoef5->text().toDouble();

    //We assign the correlation to save
    selection=ui->cbSubsToolsSelAddCorr->currentIndex();
    switch (selection){
    case 0://Cp0 DIPPR100
        corr.form=6;
        break;
    case 1://Vp Wagner 2,5
        corr.form=24;
        break;
    case 2://Vp DIPPR101
        break;
        corr.form=20;
    case 3://Boil temp. Polynomia2
        corr.form=130;
        break;
    case 4://Hv DIPPR106
        corr.form=91;
        break;
    case 5://Liquid Cp DIPPR100
        corr.form=17;
        break;
    case 6://Liq. density DIPPR106
        corr.form=47;
        break;
    case 7://Liq. viscosity DIPPR101
        corr.form=30;
        break;
    case 8://Liq. thermal cond. DIPPR100
        corr.form=50;
        break;
    case 9://Liq.surf.tension DIPPR100
        corr.form=60;
        break;
    case 10://Gas sat. density modified Wagner-Pruss
        corr.form=101;
        break;
    case 11://Gas viscosity DIPPR100
        corr.form=111;
        break;
    case 12://Gas thermal cond.DIPPR100
        corr.form=121;
        break;
    }
    AddCorrToDataBase(subs->id,&corr,&lowLimit,&highLimit,&comment,&db);
}

//Mixture calculation tab setup
//*****************************

//Pass the selected Eos and Cp0 correlation to an array format
void FreeFluidsMainWindow::getMixEosCpSel(){
    //int eosSel[6],cp0Sel[6];//the ids of the eos and cp0 correlation selected for each possible substance
    //We read the position of the eos and cp0 selection made in all available positions
    eosSel[0]=ui->cbMixCalcEosSelec1->currentIndex();
    eosSel[1]=ui->cbMixCalcEosSelec2->currentIndex();
    eosSel[2]=ui->cbMixCalcEosSelec3->currentIndex();
    eosSel[3]=ui->cbMixCalcEosSelec4->currentIndex();
    eosSel[4]=ui->cbMixCalcEosSelec5->currentIndex();
    eosSel[5]=ui->cbMixCalcEosSelec6->currentIndex();
    cp0Sel[0]=ui->cbMixCalcCp0Selec1->currentIndex();
    cp0Sel[1]=ui->cbMixCalcCp0Selec2->currentIndex();
    cp0Sel[2]=ui->cbMixCalcCp0Selec3->currentIndex();
    cp0Sel[3]=ui->cbMixCalcCp0Selec4->currentIndex();
    cp0Sel[4]=ui->cbMixCalcCp0Selec5->currentIndex();
    cp0Sel[5]=ui->cbMixCalcCp0Selec6->currentIndex();
}

//Slot for adding a new substances to the composition table
void FreeFluidsMainWindow::twMixCompositionAdd(){
    int subsId;
    QString subsName;
    double MW;
    int i=0;
    subsId=subsListModel->record(ui->cbSubsCalcSelSubs->currentIndex()).value("Id").toInt();
    subsName=subsListModel->record(ui->cbSubsCalcSelSubs->currentIndex()).value("Name").toString();
    MW=subsListModel->record(ui->cbSubsCalcSelSubs->currentIndex()).value("MW").toDouble();
    while ((ui->twMixComposition->item(i,0)->text().toInt()>0)&&(i<(ui->twMixComposition->rowCount()-1))) i++;//We look for the first free position
    //we could use numSubs, but this allows to erase a substance and refill the empty row
    ui->twMixComposition->item(i,0)->setText(QString::number(subsId));//add Id
    ui->twMixComposition->item(i,1)->setText(subsName);//add name
    ui->twMixComposition->item(i,2)->setText(QString::number(MW));//add MW
}

//Slot for clearing content in the table
void  FreeFluidsMainWindow::twMixCompositionClear(){
    int i,j;//the loop variables
    //we clear the content of the composition table
    for (i=0;i<ui->twMixComposition->rowCount();i++) for (j=0;j<ui->twMixComposition->columnCount();j++) ui->twMixComposition->item(i,j)->setText("");
    numSubs=0;
    ui->leMixCalcMWmix->setText("");
}

//Slot for calculation of mass and molar fractions in the composition table
void  FreeFluidsMainWindow::twMixCompositionCalcFract(){
    unsigned j;
    bool mass;
    numSubs=0;
    while ((ui->twMixComposition->item(numSubs,3)->text().toDouble()>0)&&(numSubs<(ui->twMixComposition->rowCount()-1))) numSubs++;//we count the filled rows
    double MW[numSubs],q[numSubs],massFrac[numSubs],moleFrac[numSubs];
    double MWmix=0;
    for (j=0;j<numSubs;j++){
        MW[j]=ui->twMixComposition->item(j,2)->text().toDouble();
        q[j]=ui->twMixComposition->item(j,3)->text().toDouble();
    }
    if (ui->cbMixCompositionMass->isChecked()==true) mass=true;
    else mass=false;
    FF_FractionsCalculation(numSubs, MW, q, mass, massFrac, moleFrac);//we obtain the fractions
    for (j=0;j<numSubs;j++){//We fill the table with the results and calculate the mix molecular weight
        ui->twMixComposition->item(j,4)->setText(QString::number(massFrac[j]));
        ui->twMixComposition->item(j,5)->setText(QString::number(moleFrac[j]));
        MWmix=MWmix+MW[j]*moleFrac[j];
    }
    ui->leMixCalcMWmix->setText(QString::number(MWmix));//We fill the molecular weight of the mixture
}

//Slot for loading the existing EOS and Cp0 correlations for the selected substances
void  FreeFluidsMainWindow::cbMixCalcEosTypeLoad(int position){
    QSqlQuery queryEos,queryCp0;
    QString eosTypeQs;
    unsigned i=0,j;
    if (position==0){
        thSys->eosType=FF_NoType;
        eosTypeQs="None";
    }
    else if (position==1){
        thSys->eosType=FF_CubicPR;
        eosTypeQs="Cubic PR";
    }
    else if (position==2){
        thSys->eosType=FF_CubicSRK;
        eosTypeQs="Cubic SRK";
    }
    else if (position==3){
        thSys->eosType=FF_SAFTtype;
        eosTypeQs="SAFT";
    }
    while ((ui->twMixComposition->item(i,0)->text().toInt()>0)&&(i<(ui->twMixComposition->rowCount()))) i++;//we count the filled rows. We could use also numSubs.
    //We load the comboboxes for eos and cp0 selection with the available options for the selected substances and eos type
    for (j=0;j<i;j++){
        //printf("Hola aqui %s\n",eos.toUtf8().constData());
        queryEos.prepare("SELECT EosParam.Eos,EosParam.Description,EosParam.Id FROM EosParam INNER JOIN Eos ON EosParam.Eos=Eos.Eos WHERE ((IdProduct=?) AND (Eos.Type = ?)) ORDER BY EosParam.Eos");
        queryEos.bindValue(0,ui->twMixComposition->item(j,0)->text().toInt());
        queryEos.bindValue(1,eosTypeQs);
        //printf("Producto:%i\n",ui->twMixComposition->item(j,0)->text().toInt());
        queryEos.exec();
        mixCalcEOSModel[j]->setQuery(queryEos);
        tvMixCalcSelEOS[j]->setColumnHidden(2,true);
        //tvMixCalcSelEOS[j]->resizeColumnsToContents();
        tvMixCalcSelEOS[j]->setColumnWidth(0,75);
        tvMixCalcSelEOS[j]->setColumnWidth(1,364);
        queryCp0.prepare("SELECT CorrelationEquations.Equation AS Equation, CorrelationParam.Reference AS Reference, CorrelationParam.Id AS Id FROM PhysProp "
                     "INNER JOIN (CorrelationEquations INNER JOIN (Correlations INNER JOIN CorrelationParam ON "
                     "Correlations.Number = CorrelationParam.NumCorrelation) ON CorrelationEquations.Id = "
                     "Correlations.IdEquation) ON PhysProp.Id = Correlations.IdPhysProp WHERE (((CorrelationParam.IdProduct)=?) "
                     "AND ((PhysProp.Property)=?)) ORDER BY Equation");

        //queryCp0.prepare("SELECT Correlations.Name, CorrelationParam.Reference, CorrelationParam.Id FROM Correlations "
        //                 "INNER JOIN CorrelationParam ON Correlations.Number = CorrelationParam.NumCorrelation WHERE "
        //                 "(((CorrelationParam.IdProduct)=?) AND ((Correlations.Type)= ?))");

        queryCp0.addBindValue(ui->twMixComposition->item(j,0)->text().toInt());
        queryCp0.addBindValue("Cp0");
        queryCp0.exec();
        mixCalcCp0Model[j]->setQuery(queryCp0);
        tvMixCalcSelCp0[j]->setColumnHidden(2,true);
        tvMixCalcSelCp0[j]->setColumnWidth(0,75);
        tvMixCalcSelCp0[j]->setColumnWidth(1,227);
        //tvMixCalcSelCp0[j]->resizeColumnsToContents();
        //ui->lineEdit->setText(subsListModel->record(position).value("Name").toString());
    }
}

//Slot for storing the selected mixing rule
void FreeFluidsMainWindow::cbMixCalcMixRuleLoad(int row){
    switch(row){//We charge the selecte mixing rule
    case 0:
        rule=FF_NoMixRul;
        break;
    case 1:
        rule=FF_VdW;
        break;
    case 2:
        rule=FF_PR;
        break;
    case 3:
        rule=FF_MKP;
        break;
    }
}

//Slot for displaying the intParam array for eos, for the selecte pair, and charge the combobox of available interaction parameters
void FreeFluidsMainWindow::twMixIntParamEosDisplay(int row,int column){
    //We display in the table the actual interaction parameters
    for(int i=0;i<6;i++){
        ui->twMixIntParamEos->item(0,i)->setText(QString::number(pintParamEos[row][column][i]));
        ui->twMixIntParamEos->item(1,i)->setText(QString::number(pintParamEos[column][row][i]));
    }
    //We update the combobox with the available interactions parameters available to choose for the pair
    QString ruleString;
    QSqlQuery queryIntParam;
    switch(rule){
    case FF_NoMixRul:
        ruleString="None";
        break;
    case FF_VdW:
        ruleString="VdW";
        break;
    case FF_PR:
        ruleString="PR";
        break;
    case FF_MKP:
        ruleString="MKP";
        break;
    }
    queryIntParam.prepare("SELECT EosInteraction.* FROM EosInteraction  WHERE ((IdProduct1=?) AND (IdProduct2=?) AND (EosType=?) AND (EosInteraction.Rule=?))");
    queryIntParam.bindValue(0,ui->twMixComposition->item(row,0)->text().toInt());
    queryIntParam.bindValue(1,ui->twMixComposition->item(column,0)->text().toInt());
    queryIntParam.bindValue(2,thSys->eosType);
    queryIntParam.bindValue(3,ruleString);
    queryIntParam.exec();
    mixIntParamSelModel->setQuery(queryIntParam);
    tvMixIntParamSel->setColumnWidth(0,1);
    tvMixIntParamSel->setColumnWidth(20,200);
    //tvMixIntParamSel->setColumnHidden(0,true);
    for(int i=6;i<20;i++) tvMixIntParamSel->setColumnHidden(i,true);
    ui->cbMixIntParamSel->setModelColumn(20);
}

//Slot for filling the intParam array, and tablewidget, for eos, from the database
void FreeFluidsMainWindow::twMixIntParamEosFill(int row){
    int i,j;
    i=ui->twMixPairSel->currentRow();
    j=ui->twMixPairSel->currentColumn();
    //update array
    pintParamEos[i][j][0]=mixIntParamSelModel->record(row).value("Param1").toFloat();
    pintParamEos[i][j][1]=mixIntParamSelModel->record(row).value("Param2").toFloat();
    pintParamEos[i][j][2]=mixIntParamSelModel->record(row).value("Param3").toFloat();
    pintParamEos[i][j][3]=mixIntParamSelModel->record(row).value("Param4").toFloat();
    pintParamEos[i][j][4]=mixIntParamSelModel->record(row).value("Param5").toFloat();
    pintParamEos[i][j][5]=mixIntParamSelModel->record(row).value("Param6").toFloat();

    pintParamEos[j][i][0]=mixIntParamSelModel->record(row).value("Param1i").toFloat();
    pintParamEos[j][i][1]=mixIntParamSelModel->record(row).value("Param2i").toFloat();
    pintParamEos[j][i][2]=mixIntParamSelModel->record(row).value("Param3i").toFloat();
    pintParamEos[j][i][3]=mixIntParamSelModel->record(row).value("Param4i").toFloat();
    pintParamEos[j][i][4]=mixIntParamSelModel->record(row).value("Param5i").toFloat();
    pintParamEos[j][i][5]=mixIntParamSelModel->record(row).value("Param6i").toFloat();
    //update tablewidget for see the parameters
    for(int k=0;k<6;k++){
        ui->twMixIntParamEos->item(0,k)->setText(QString::number(pintParamEos[i][j][k]));
        ui->twMixIntParamEos->item(1,k)->setText(QString::number(pintParamEos[j][i][k]));
    }
}

//Slot for updating the interaction parameter array for eos, from the display
void FreeFluidsMainWindow::twMixIntParamEosUpdate(int row,int column){
    int i,j;
    if (column<6){
        i=ui->twMixPairSel->currentRow();
        j=ui->twMixPairSel->currentColumn();
        if (row==0) pintParamEos[i][j][column]=ui->twMixIntParamEos->item(row,column)->text().toFloat();
        else if (row==1) pintParamEos[j][i][column]=ui->twMixIntParamEos->item(row,column)->text().toFloat();
        //printf("%f\n",pintParamEos[i][j][column]);
    }
}

//Slot for mixture calculation, and display in table
void FreeFluidsMainWindow::twMixCalcUpdate(){
    FreeFluidsMainWindow::getMixEosCpSel();//we read the selected Eos and Cp index for each substance
    int j;//the loop variable
    for (j=0;j<ui->twMixCalc->rowCount();j++) ui->twMixCalc->item(j,0)->setText("");//we clear the content of the results table
    int numIntParam=numSubs*numSubs*6;//Number of interaction parameters
    int subsId[numSubs],eosId[numSubs],cp0Id[numSubs];//the ids of the substance, eos selected, and cp0 correlation, only for the defined substances
    QString eosModel[numSubs];//The eos model to use for each defined substance, in text format
    enum FF_EOS eos[numSubs];//The eos model to use for each defined substance, in enum format
    double c[numSubs];//will hold the global composition as molar fraction
    double pintParam[numIntParam];//will hold the interaction parameters between substances
    //Now we need to read the selections made for: the Id of the substance,the molar fraction, the EOS and the Cp0 formula to use. And the type of EOS
    for (j=0;j<numSubs;j++){
        //thSys->substance.baseProp.id=ui->twMixComposition->item(j,0)->text().toInt();//Substance Id
        //thSys->cp0Corr->id=mixCalcCp0Model[j]->record(cp0Sel[j]).value("Id").toInt();

        subsId[j]=ui->twMixComposition->item(j,0)->text().toInt();//Substance Id
        c[j]=ui->twMixComposition->item(j,5)->text().toDouble();//substance molar fraction
        eosId[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Id").toInt();
        eosModel[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Eos").toString();
        cp0Id[j]=mixCalcCp0Model[j]->record(cp0Sel[j]).value("Id").toInt();
        if (!((subsId[j]>0)&&(eosId[j]>0))) return;//Without substance or eos calculation is impossible
        ConvertEosToEnumeration(&eosModel[j],&eos[j]);//Now it is necessary to put the eos we are using in enumeration format
        //std::cout<<subsId[j]<<" "<<eosId[j]<<" "<<cp0Id[j]<<" "<<eosModel[j].toStdString()<<" "<<eos[j]<<std::endl;
    }
    for (int j=0;j<numIntParam;j++) pintParam[j]=0.0;//by now we fill with 0 all the interactionparameters

    //Now we begin the calculations
    FF_Correlation cp0[numSubs];
    FF_ThermoProperties th0,thR;//here will be stored the result of the calculations for the global mixture
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will recive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    QString phase;
    double Z;
    double bubbleT;
    double yBubble[numSubs];
    double ArrDerivatives[6];
    FF_SaftEOSdata dataP[numSubs];
    FF_SWEOSdata  dataS[numSubs];
    FF_CubicEOSdata dataC[numSubs];
    FF_CubicParam param[numSubs];
    th0.T=thR.T=ui->leMixCalcTemp->text().toDouble()+273.15;//we read the selected temperature
    thR.P=1e5*ui->leMixCalcPres->text().toDouble();//we read the selected pressure
    switch(eos[0]){
    case FF_PCSAFT:
    case FF_DPCSAFT_GV:
    case FF_DPCSAFT_JC:
    case FF_PCSAFTPOL1:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataP[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //std::cout<<eos[j]<<" "<<dataP[j].Tc<<" "<<dataP[j].Pc<<" "<<dataP[j].sigma<<" "<<dataP[j].m<<std::endl;
            //printf("cp0Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
            //GetEosData2();
        }
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        break;
    case FF_SW:
    case FF_IAPWS95:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataS[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        break;
    default:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataC[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("Tc:%f Pc:%f w:%f k1:%f k2:%f k3:%f\n",dataC[j].Tc,dataC[j].Pc,dataC[j].w,dataC[j].k1,dataC[j].k2,dataC[j].k3);
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        break;
    }  
    phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
    phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
    if (state=='f') phase="Calc.fail";
    else if ((state=='l')||(state=='g')||(state=='b')) phase="Unique";
    else if (state=='L') phase="Liquid";
    else if (state=='G') phase="Gas";
    else if (state=='E') phase="Equilibrium";
    if ((state=='l')||(state=='L')||(state=='b')){
        thR.V=answerL[0];
        Z=answerL[2];
    }
    else if ((state=='g')||(state=='G')){
        thR.V=answerG[0];
        Z=answerG[2];
    }
    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thR.T,thR.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);
    th0.V=thR.V;
    FF_MixFF_IdealThermoEOS(&numSubs,cp0,c,&refT,&refP,&th0);
    switch(eos[0]){
    case FF_PCSAFT:
    case FF_DPCSAFT_GV:
    case FF_DPCSAFT_JC:
    case FF_PCSAFTPOL1:
        thSys->MixThermoEOS(c,&thR);
        break;
    case FF_SW:
    case FF_IAPWS95:
        break;
    default:
        thSys->MixThermoEOS(c,&thR);
        //FF_BubbleT(eos,&rule,&thR.P,&numSubs,dataC,pintParam,c,&bubbleT,yBubble);
        printf("BubbleT:%f, %f %f\n",bubbleT,yBubble[0],yBubble[1]);
        break;
    }
    ui->twMixCalc->item(0,0)->setText(QString::number(MW));
    ui->twMixCalc->item(1,0)->setText(QString::number(thR.P*0.00001));
    ui->twMixCalc->item(2,0)->setText(QString::number(thR.T-273.15));
    ui->twMixCalc->item(3,0)->setText(phase);
    ui->twMixCalc->item(4,0)->setText(QString::number(phiL));
    ui->twMixCalc->item(5,0)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,0)->setText(QString::number(Z));
    ui->twMixCalc->item(7,0)->setText(QString::number(thR.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,0)->setText(QString::number(MW/thR.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,0)->setText(QString::number(th0.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,0)->setText(QString::number(th0.S/MW));
    ui->twMixCalc->item(11,0)->setText(QString::number(th0.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,0)->setText(QString::number(th0.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,0)->setText(QString::number(thR.H/MW));
    ui->twMixCalc->item(14,0)->setText(QString::number(thR.U/MW));
    ui->twMixCalc->item(15,0)->setText(QString::number(thR.S/MW));
    ui->twMixCalc->item(16,0)->setText(QString::number(thR.Cp/MW));
    ui->twMixCalc->item(17,0)->setText(QString::number(thR.Cv/MW));
    ui->twMixCalc->item(18,0)->setText(QString::number(thR.SS));
    ui->twMixCalc->item(19,0)->setText(QString::number(thR.JT*1e5));
    ui->twMixCalc->item(20,0)->setText(QString::number(thR.IT*1e2));
    ui->twMixCalc->item(21,0)->setText(QString::number(thR.dP_dT*1e-5));
    ui->twMixCalc->item(22,0)->setText(QString::number(thR.dP_dV*1e-5));

}

//Slot for mixture bubble T calculation, and display in table
void FreeFluidsMainWindow::twMixCalcBubbleT(){
    FreeFluidsMainWindow::getMixEosCpSel();//we read the selected Eos and Cp index for each substance
    int j;//the loop variable
    for (j=0;j<ui->twMixCalc->rowCount();j++){//we clear the content of the results table
        ui->twMixCalc->item(j,1)->setText("");
        ui->twMixCalc->item(j,2)->setText("");
    }
    //int numIntParam=numSubs*numSubs*6;//Number of interaction parameters. numSubs has been counted when calculating mass and mole fractions
    int subsId[numSubs],eosId[numSubs],cp0Id[numSubs];//the ids of the substance, eos selected, and cp0 correlation, only for the defined substances
    QString eosModel[numSubs];//The eos model to use for each defined substance, in text format
    enum FF_EOS eos[numSubs];//The eos model to use for each defined substance, in enum format
    double c[numSubs];//will hold the global composition as molar fraction
    double pintParam[numSubs][numSubs][6];//will hold the interaction parameters between substances
    //Now we need to read the selections made for: the Id of the substance,the molar fraction, the EOS and the Cp0 formula to use. And the type of EOS
    for (j=0;j<numSubs;j++){
        subsId[j]=ui->twMixComposition->item(j,0)->text().toInt();//Substance Id
        c[j]=ui->twMixComposition->item(j,5)->text().toDouble();//substance molar fraction
        eosId[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Id").toInt();
        eosModel[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Eos").toString();
        cp0Id[j]=mixCalcCp0Model[j]->record(cp0Sel[j]).value("Id").toInt();
        if (!((subsId[j]>0)&&(eosId[j]>0))) return;//Without substance or eos calculation is impossible
        ConvertEosToEnumeration(&eosModel[j],&eos[j]);//Now it is necessary to put the eos we are using in enumeration format
        //std::cout<<subsId[j]<<" "<<eosId[j]<<" "<<cp0Id[j]<<" "<<eosModel[j].toStdString()<<" "<<eos[j]<<std::endl;
    }
    //we fill all the interaction parameters,from the selection made in screen
    for(int i=0;i<numSubs;i++) for(int j=0;j<numSubs;j++) for(int k=0;k<6;k++) pintParam[i][j][k]=pintParamEos[i][j][k];

    //Now we begin the calculations
     FF_Correlation cp0[numSubs];
     FF_ThermoProperties th0,thR,th0g,thRg;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    QString phase;
    double Z,Zg;
    double bubbleT;
    double bubbleTguess=0;
    double yBubble[numSubs];
    double substPhiL[numSubs];
    double substPhiG[numSubs];
    double ArrDerivatives[6];
     FF_SaftEOSdata dataP[numSubs];
     FF_SWEOSdata  dataS[numSubs];
     FF_CubicEOSdata dataC[numSubs];
     FF_CubicParam param[numSubs];
    thR.P=thRg.P=1e5*ui->leMixCalcPres->text().toDouble();//we read the selected pressure
    switch(thSys->eosType){
    case FF_SAFTtype:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataP[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //std::cout<<eos[j]<<" "<<dataP[j].Tc<<" "<<dataP[j].Pc<<" "<<dataP[j].sigma<<" "<<dataP[j].m<<std::endl;
            //printf("cp0Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->BubbleT(&thR.P,c,&bubbleTguess,&bubbleT,yBubble,substPhiL,substPhiG);
        th0.T=thR.T=th0g.T=thRg.T=bubbleT;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(c,&th0);
        thSys->MixThermoEOS(c,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,yBubble,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        FF_MixFF_IdealThermoEOS(&numSubs,cp0,yBubble,&refT,&refP,&th0g);
        thSys->MixThermoEOS(yBubble,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    case FF_SWtype:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataS[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        break;
    default:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataC[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("Tc:%f Pc:%f w:%f k1:%f k2:%f k3:%f\n",dataC[j].Tc,dataC[j].Pc,dataC[j].w,dataC[j].k1,dataC[j].k2,dataC[j].k3);
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->BubbleT(&thR.P,c,&bubbleTguess,&bubbleT,yBubble,substPhiL,substPhiG);
        th0.T=thR.T=th0g.T=thRg.T=bubbleT;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(c,&th0);
        thSys->MixThermoEOS(c,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,yBubble,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        thSys->MixFF_IdealThermoEOS(yBubble,&th0g);
        thSys->MixThermoEOS(yBubble,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    }


    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thR.T,thR.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);

    ui->twMixCalc->item(1,1)->setText(QString::number(thRg.P*0.00001));
    ui->twMixCalc->item(2,1)->setText(QString::number(bubbleT-273.15));
    ui->twMixCalc->item(3,1)->setText(QString::number(0));
    //ui->twMixCalc->item(4,1)->setText(QString::number(phiL));
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));
    ui->twMixCalc->item(7,1)->setText(QString::number(thRg.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,1)->setText(QString::number(MW/thRg.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,1)->setText(QString::number(th0g.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,1)->setText(QString::number(th0g.S/MW));
    ui->twMixCalc->item(11,1)->setText(QString::number(th0g.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,1)->setText(QString::number(th0g.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,1)->setText(QString::number(thRg.H/MW));
    ui->twMixCalc->item(14,1)->setText(QString::number(thRg.U/MW));
    ui->twMixCalc->item(15,1)->setText(QString::number(thRg.S/MW));
    ui->twMixCalc->item(16,1)->setText(QString::number(thRg.Cp/MW));
    ui->twMixCalc->item(17,1)->setText(QString::number(thRg.Cv/MW));
    ui->twMixCalc->item(18,1)->setText(QString::number(thRg.SS));
    ui->twMixCalc->item(19,1)->setText(QString::number(thRg.JT*1e5));
    ui->twMixCalc->item(20,1)->setText(QString::number(thRg.IT*1e2));
    ui->twMixCalc->item(21,1)->setText(QString::number(thRg.dP_dT*1e-5));
    ui->twMixCalc->item(22,1)->setText(QString::number(thRg.dP_dV*1e-5));
    ui->twMixCalc->item(29,1)->setText(QString::number(yBubble[0]));
    ui->twMixCalc->item(30,1)->setText(QString::number(yBubble[1]));
    ui->twMixCalc->item(41,1)->setText(QString::number(substPhiG[0]));
    ui->twMixCalc->item(42,1)->setText(QString::number(substPhiG[1]));

    ui->twMixCalc->item(1,2)->setText(QString::number(thR.P*0.00001));
    ui->twMixCalc->item(2,2)->setText(QString::number(bubbleT-273.15));
    ui->twMixCalc->item(3,2)->setText(QString::number(1));
    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    //ui->twMixCalc->item(5,2)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
    ui->twMixCalc->item(7,2)->setText(QString::number(thR.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,2)->setText(QString::number(MW/thR.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,2)->setText(QString::number(th0.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,2)->setText(QString::number(th0.S/MW));
    ui->twMixCalc->item(11,2)->setText(QString::number(th0.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,2)->setText(QString::number(th0.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,2)->setText(QString::number(thR.H/MW));
    ui->twMixCalc->item(14,2)->setText(QString::number(thR.U/MW));
    ui->twMixCalc->item(15,2)->setText(QString::number(thR.S/MW));
    ui->twMixCalc->item(16,2)->setText(QString::number(thR.Cp/MW));
    ui->twMixCalc->item(17,2)->setText(QString::number(thR.Cv/MW));
    ui->twMixCalc->item(18,2)->setText(QString::number(thR.SS));
    ui->twMixCalc->item(19,2)->setText(QString::number(thR.JT*1e5));
    ui->twMixCalc->item(20,2)->setText(QString::number(thR.IT*1e2));
    ui->twMixCalc->item(21,2)->setText(QString::number(thR.dP_dT*1e-5));
    ui->twMixCalc->item(22,2)->setText(QString::number(thR.dP_dV*1e-5));
    ui->twMixCalc->item(29,2)->setText(QString::number(c[0]));
    ui->twMixCalc->item(30,2)->setText(QString::number(c[1]));
    ui->twMixCalc->item(41,2)->setText(QString::number(substPhiL[0]));
    ui->twMixCalc->item(42,2)->setText(QString::number(substPhiL[1]));
}

    //Slot for mixture dew T calculation, and display in table
void FreeFluidsMainWindow::twMixCalcDewT(){
    FreeFluidsMainWindow::getMixEosCpSel();//we read the selected Eos and Cp index for each substance
    int j;//the loop variable
    for (j=0;j<ui->twMixCalc->rowCount();j++){//we clear the content of the results table
        ui->twMixCalc->item(j,1)->setText("");
        ui->twMixCalc->item(j,2)->setText("");
    }
    int subsId[numSubs],eosId[numSubs],cp0Id[numSubs];//the ids of the substance, eos selected, and cp0 correlation, only for the defined substances
    QString eosModel[numSubs];//The eos model to use for each defined substance, in text format
    enum FF_EOS eos[numSubs];//The eos model to use for each defined substance, in enum format
    double c[numSubs];//will hold the global composition as molar fraction
    double pintParam[numSubs][numSubs][6];//will hold the interaction parameters between substances
    //Now we need to read the selections made for: the Id of the substance,the molar fraction, the EOS and the Cp0 formula to use. And the type of EOS
    for (j=0;j<numSubs;j++){
        subsId[j]=ui->twMixComposition->item(j,0)->text().toInt();//Substance Id
        c[j]=ui->twMixComposition->item(j,5)->text().toDouble();//substance molar fraction
        eosId[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Id").toInt();
        eosModel[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Eos").toString();
        cp0Id[j]=mixCalcCp0Model[j]->record(cp0Sel[j]).value("Id").toInt();
        if (!((subsId[j]>0)&&(eosId[j]>0))) return;//Without substance or eos calculation is impossible
        ConvertEosToEnumeration(&eosModel[j],&eos[j]);//Now it is necessary to put the eos we are using in enumeration format
        //std::cout<<subsId[j]<<" "<<eosId[j]<<" "<<cp0Id[j]<<" "<<eosModel[j].toStdString()<<" "<<eos[j]<<std::endl;
    }
    //we fill all the interaction parameters,from the selection made in screen
    for(int i=0;i<numSubs;i++) for(int j=0;j<numSubs;j++) for(int k=0;k<6;k++) pintParam[i][j][k]=pintParamEos[i][j][k];

    //Now we begin the calculations
     FF_Correlation cp0[numSubs];
     FF_ThermoProperties th0,thR,th0g,thRg;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    QString phase;
    double Z,Zg;
    double dewT;
    double dewTguess=0;
    double xDew[numSubs];
    double substPhiL[numSubs];
    double substPhiG[numSubs];
    double ArrDerivatives[6];
     FF_SaftEOSdata dataP[numSubs];
     FF_SWEOSdata  dataS[numSubs];
     FF_CubicEOSdata dataC[numSubs];
     FF_CubicParam param[numSubs];
    thR.P=thRg.P=1e5*ui->leMixCalcPres->text().toDouble();//we read the selected pressure
    switch(thSys->eosType){
    case FF_SAFTtype:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataP[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //std::cout<<eos[j]<<" "<<dataP[j].Tc<<" "<<dataP[j].Pc<<" "<<dataP[j].sigma<<" "<<dataP[j].m<<std::endl;
            //printf("cp0Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->DewT(&thR.P,c,&dewTguess,&dewT,xDew,substPhiL,substPhiG);
        th0.T=thR.T=th0g.T=thRg.T=dewT;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(c,&th0);
        thSys->MixThermoEOS(c,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,xDew,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        thSys->MixFF_IdealThermoEOS(xDew,&th0g);
        thSys->MixThermoEOS(xDew,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    case FF_SWtype:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataS[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        break;
    default:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataC[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("Tc:%f Pc:%f w:%f k1:%f k2:%f k3:%f\n",dataC[j].Tc,dataC[j].Pc,dataC[j].w,dataC[j].k1,dataC[j].k2,dataC[j].k3);
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->DewT(&thR.P,c,&dewTguess,&dewT,xDew,substPhiL,substPhiG);
        th0.T=thR.T=th0g.T=thRg.T=dewT;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,xDew,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(xDew,&th0);
        thSys->MixThermoEOS(xDew,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,c,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        thSys->MixFF_IdealThermoEOS(c,&th0g);
        thSys->MixThermoEOS(c,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    }


    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thR.T,thR.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);

    ui->twMixCalc->item(1,1)->setText(QString::number(thRg.P*0.00001));
    ui->twMixCalc->item(2,1)->setText(QString::number(dewT-273.15));
    ui->twMixCalc->item(3,1)->setText(QString::number(1));
    //ui->twMixCalc->item(4,1)->setText(QString::number(phiL));
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));
    ui->twMixCalc->item(7,1)->setText(QString::number(thRg.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,1)->setText(QString::number(MW/thRg.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,1)->setText(QString::number(th0g.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,1)->setText(QString::number(th0g.S/MW));
    ui->twMixCalc->item(11,1)->setText(QString::number(th0g.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,1)->setText(QString::number(th0g.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,1)->setText(QString::number(thRg.H/MW));
    ui->twMixCalc->item(14,1)->setText(QString::number(thRg.U/MW));
    ui->twMixCalc->item(15,1)->setText(QString::number(thRg.S/MW));
    ui->twMixCalc->item(16,1)->setText(QString::number(thRg.Cp/MW));
    ui->twMixCalc->item(17,1)->setText(QString::number(thRg.Cv/MW));
    ui->twMixCalc->item(18,1)->setText(QString::number(thRg.SS));
    ui->twMixCalc->item(19,1)->setText(QString::number(thRg.JT*1e5));
    ui->twMixCalc->item(20,1)->setText(QString::number(thRg.IT*1e2));
    ui->twMixCalc->item(21,1)->setText(QString::number(thRg.dP_dT*1e-5));
    ui->twMixCalc->item(22,1)->setText(QString::number(thRg.dP_dV*1e-5));
    ui->twMixCalc->item(29,1)->setText(QString::number(c[0]));
    ui->twMixCalc->item(30,1)->setText(QString::number(c[1]));
    ui->twMixCalc->item(41,1)->setText(QString::number(substPhiG[0]));
    ui->twMixCalc->item(42,1)->setText(QString::number(substPhiG[1]));

    ui->twMixCalc->item(1,2)->setText(QString::number(thR.P*0.00001));
    ui->twMixCalc->item(2,2)->setText(QString::number(dewT-273.15));
    ui->twMixCalc->item(3,2)->setText(QString::number(0));
    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    //ui->twMixCalc->item(5,2)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
    ui->twMixCalc->item(7,2)->setText(QString::number(thR.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,2)->setText(QString::number(MW/thR.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,2)->setText(QString::number(th0.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,2)->setText(QString::number(th0.S/MW));
    ui->twMixCalc->item(11,2)->setText(QString::number(th0.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,2)->setText(QString::number(th0.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,2)->setText(QString::number(thR.H/MW));
    ui->twMixCalc->item(14,2)->setText(QString::number(thR.U/MW));
    ui->twMixCalc->item(15,2)->setText(QString::number(thR.S/MW));
    ui->twMixCalc->item(16,2)->setText(QString::number(thR.Cp/MW));
    ui->twMixCalc->item(17,2)->setText(QString::number(thR.Cv/MW));
    ui->twMixCalc->item(18,2)->setText(QString::number(thR.SS));
    ui->twMixCalc->item(19,2)->setText(QString::number(thR.JT*1e5));
    ui->twMixCalc->item(20,2)->setText(QString::number(thR.IT*1e2));
    ui->twMixCalc->item(21,2)->setText(QString::number(thR.dP_dT*1e-5));
    ui->twMixCalc->item(22,2)->setText(QString::number(thR.dP_dV*1e-5));
    ui->twMixCalc->item(29,2)->setText(QString::number(xDew[0]));
    ui->twMixCalc->item(30,2)->setText(QString::number(xDew[1]));
    ui->twMixCalc->item(41,2)->setText(QString::number(substPhiL[0]));
    ui->twMixCalc->item(42,2)->setText(QString::number(substPhiL[1]));
}

//Slot for mixture bubble P calculation, and display in table
void FreeFluidsMainWindow::twMixCalcBubbleP(){
    FreeFluidsMainWindow::getMixEosCpSel();//we read the selected Eos and Cp index for each substance
    int j;//the loop variable
    for (j=0;j<ui->twMixCalc->rowCount();j++){//we clear the content of the results table
        ui->twMixCalc->item(j,1)->setText("");
        ui->twMixCalc->item(j,2)->setText("");
    }
    int subsId[numSubs],eosId[numSubs],cp0Id[numSubs];//the ids of the substance, eos selected, and cp0 correlation, only for the defined substances
    QString eosModel[numSubs];//The eos model to use for each defined substance, in text format
    enum FF_EOS eos[numSubs];//The eos model to use for each defined substance, in enum format
    double c[numSubs];//will hold the global composition as molar fraction
    double pintParam[numSubs][numSubs][6];//will hold the interaction parameters between substances
    //Now we need to read the selections made for: the Id of the substance,the molar fraction, the EOS and the Cp0 formula to use. And the type of EOS
    for (j=0;j<numSubs;j++){
        subsId[j]=ui->twMixComposition->item(j,0)->text().toInt();//Substance Id
        c[j]=ui->twMixComposition->item(j,5)->text().toDouble();//substance molar fraction
        eosId[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Id").toInt();
        eosModel[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Eos").toString();
        cp0Id[j]=mixCalcCp0Model[j]->record(cp0Sel[j]).value("Id").toInt();
        if (!((subsId[j]>0)&&(eosId[j]>0))) return;//Without substance or eos calculation is impossible
        ConvertEosToEnumeration(&eosModel[j],&eos[j]);//Now it is necessary to put the eos we are using in enumeration format
        //std::cout<<subsId[j]<<" "<<eosId[j]<<" "<<cp0Id[j]<<" "<<eosModel[j].toStdString()<<" "<<eos[j]<<std::endl;
    }
    //we fill all the interaction parameters,from the selection made in screen
    for(int i=0;i<numSubs;i++) for(int j=0;j<numSubs;j++) for(int k=0;k<6;k++) pintParam[i][j][k]=pintParamEos[i][j][k];
    //Now we begin the calculations
     FF_Correlation cp0[numSubs];
     FF_ThermoProperties th0,thR,th0g,thRg;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    QString phase;
    double Z,Zg;
    double bubbleP;
    double bubblePguess=0;
    double yBubble[numSubs];
    double substPhiL[numSubs];
    double substPhiG[numSubs];
    double ArrDerivatives[6];
     FF_SaftEOSdata dataP[numSubs];
     FF_SWEOSdata  dataS[numSubs];
     FF_CubicEOSdata dataC[numSubs];
     FF_CubicParam param[numSubs];
    thR.T=thRg.T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    switch(thSys->eosType){
    case FF_SAFTtype:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataP[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //std::cout<<eos[j]<<" "<<dataP[j].Tc<<" "<<dataP[j].Pc<<" "<<dataP[j].sigma<<" "<<dataP[j].m<<std::endl;
            //printf("cp0Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->BubbleP(&thR.T,c,&bubblePguess,&bubbleP,yBubble,substPhiL,substPhiG);
        th0.P=thR.P=th0g.P=thRg.P=bubbleP;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(c,&th0);
        thSys->MixThermoEOS(c,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,yBubble,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        thSys->MixFF_IdealThermoEOS(yBubble,&th0g);
        thSys->MixThermoEOS(yBubble,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    case FF_SW:
    case FF_IAPWS95:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataS[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        break;
    default:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataC[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("Tc:%f Pc:%f w:%f k1:%f k2:%f k3:%f\n",dataC[j].Tc,dataC[j].Pc,dataC[j].w,dataC[j].k1,dataC[j].k2,dataC[j].k3);
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->BubbleP(&thR.T,c,&bubblePguess,&bubbleP,yBubble,substPhiL,substPhiG);
        th0.P=thR.P=th0g.P=thRg.P=bubbleP;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(c,&th0);
        thSys->MixThermoEOS(c,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,yBubble,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        thSys->MixFF_IdealThermoEOS(yBubble,&th0g);
        thSys->MixThermoEOS(yBubble,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    }
    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thR.T,thR.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);

    ui->twMixCalc->item(1,1)->setText(QString::number(thRg.P*0.00001));
    ui->twMixCalc->item(2,1)->setText(QString::number(thRg.T-273.15));
    ui->twMixCalc->item(3,1)->setText(QString::number(0));
    //ui->twMixCalc->item(4,1)->setText(QString::number(phiL));
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));
    ui->twMixCalc->item(7,1)->setText(QString::number(thRg.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,1)->setText(QString::number(MW/thRg.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,1)->setText(QString::number(th0g.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,1)->setText(QString::number(th0g.S/MW));
    ui->twMixCalc->item(11,1)->setText(QString::number(th0g.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,1)->setText(QString::number(th0g.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,1)->setText(QString::number(thRg.H/MW));
    ui->twMixCalc->item(14,1)->setText(QString::number(thRg.U/MW));
    ui->twMixCalc->item(15,1)->setText(QString::number(thRg.S/MW));
    ui->twMixCalc->item(16,1)->setText(QString::number(thRg.Cp/MW));
    ui->twMixCalc->item(17,1)->setText(QString::number(thRg.Cv/MW));
    ui->twMixCalc->item(18,1)->setText(QString::number(thRg.SS));
    ui->twMixCalc->item(19,1)->setText(QString::number(thRg.JT*1e5));
    ui->twMixCalc->item(20,1)->setText(QString::number(thRg.IT*1e2));
    ui->twMixCalc->item(21,1)->setText(QString::number(thRg.dP_dT*1e-5));
    ui->twMixCalc->item(22,1)->setText(QString::number(thRg.dP_dV*1e-5));
    ui->twMixCalc->item(29,1)->setText(QString::number(yBubble[0]));
    ui->twMixCalc->item(30,1)->setText(QString::number(yBubble[1]));
    ui->twMixCalc->item(41,1)->setText(QString::number(substPhiG[0]));
    ui->twMixCalc->item(42,1)->setText(QString::number(substPhiG[1]));

    ui->twMixCalc->item(1,2)->setText(QString::number(thR.P*0.00001));
    ui->twMixCalc->item(2,2)->setText(QString::number(thR.T-273.15));
    ui->twMixCalc->item(3,2)->setText(QString::number(1));
    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    //ui->twMixCalc->item(5,2)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
    ui->twMixCalc->item(7,2)->setText(QString::number(thR.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,2)->setText(QString::number(MW/thR.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,2)->setText(QString::number(th0.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,2)->setText(QString::number(th0.S/MW));
    ui->twMixCalc->item(11,2)->setText(QString::number(th0.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,2)->setText(QString::number(th0.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,2)->setText(QString::number(thR.H/MW));
    ui->twMixCalc->item(14,2)->setText(QString::number(thR.U/MW));
    ui->twMixCalc->item(15,2)->setText(QString::number(thR.S/MW));
    ui->twMixCalc->item(16,2)->setText(QString::number(thR.Cp/MW));
    ui->twMixCalc->item(17,2)->setText(QString::number(thR.Cv/MW));
    ui->twMixCalc->item(18,2)->setText(QString::number(thR.SS));
    ui->twMixCalc->item(19,2)->setText(QString::number(thR.JT*1e5));
    ui->twMixCalc->item(20,2)->setText(QString::number(thR.IT*1e2));
    ui->twMixCalc->item(21,2)->setText(QString::number(thR.dP_dT*1e-5));
    ui->twMixCalc->item(22,2)->setText(QString::number(thR.dP_dV*1e-5));
    ui->twMixCalc->item(29,2)->setText(QString::number(c[0]));
    ui->twMixCalc->item(30,2)->setText(QString::number(c[1]));
    ui->twMixCalc->item(41,2)->setText(QString::number(substPhiL[0]));
    ui->twMixCalc->item(42,2)->setText(QString::number(substPhiL[1]));
}

//Slot for mixture dew P calculation, and display in table
void FreeFluidsMainWindow::twMixCalcDewP(){
    FreeFluidsMainWindow::getMixEosCpSel();//we read the selected Eos and Cp index for each substance
    int j;//the loop variable
    for (j=0;j<ui->twMixCalc->rowCount();j++){//we clear the content of the results table
        ui->twMixCalc->item(j,1)->setText("");
        ui->twMixCalc->item(j,2)->setText("");
    }
    int subsId[numSubs],eosId[numSubs],cp0Id[numSubs];//the ids of the substance, eos selected, and cp0 correlation, only for the defined substances
    QString eosModel[numSubs];//The eos model to use for each defined substance, in text format
    enum FF_EOS eos[numSubs];//The eos model to use for each defined substance, in enum format
    double c[numSubs];//will hold the global composition as molar fraction
    double pintParam[numSubs][numSubs][6];//will hold the interaction parameters between substances
    //Now we need to read the selections made for: the Id of the substance,the molar fraction, the EOS and the Cp0 formula to use. And the type of EOS
    for (j=0;j<numSubs;j++){
        subsId[j]=ui->twMixComposition->item(j,0)->text().toInt();//Substance Id
        c[j]=ui->twMixComposition->item(j,5)->text().toDouble();//substance molar fraction
        eosId[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Id").toInt();
        eosModel[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Eos").toString();
        cp0Id[j]=mixCalcCp0Model[j]->record(cp0Sel[j]).value("Id").toInt();
        if (!((subsId[j]>0)&&(eosId[j]>0))) return;//Without substance or eos calculation is impossible
        ConvertEosToEnumeration(&eosModel[j],&eos[j]);//Now it is necessary to put the eos we are using in enumeration format
        //std::cout<<subsId[j]<<" "<<eosId[j]<<" "<<cp0Id[j]<<" "<<eosModel[j].toStdString()<<" "<<eos[j]<<std::endl;
    }
    //we fill all the interaction parameters,from the selection made in screen
    for(int i=0;i<numSubs;i++) for(int j=0;j<numSubs;j++) for(int k=0;k<6;k++) pintParam[i][j][k]=pintParamEos[i][j][k];
    //Now we begin the calculations
     FF_Correlation cp0[numSubs];
     FF_ThermoProperties th0,thR,th0g,thRg;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    QString phase;
    double Z,Zg;
    double dewP;
    double dewPguess=0;
    double xDew[numSubs];
    double substPhiL[numSubs];
    double substPhiG[numSubs];
    double ArrDerivatives[6];
     FF_SaftEOSdata dataP[numSubs];
     FF_SWEOSdata  dataS[numSubs];
     FF_CubicEOSdata dataC[numSubs];
     FF_CubicParam param[numSubs];
    thR.T=thRg.T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    switch(thSys->eosType){
    case FF_SAFTtype:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataP[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //std::cout<<eos[j]<<" "<<dataP[j].Tc<<" "<<dataP[j].Pc<<" "<<dataP[j].sigma<<" "<<dataP[j].m<<std::endl;
            //printf("cp0Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->DewP(&thR.T,c,&dewPguess,&dewP,xDew,substPhiL,substPhiG);
        th0.P=thR.P=th0g.P=thRg.P=dewP;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,c,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(c,&th0);
        thSys->MixThermoEOS(c,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,xDew,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        thSys->MixFF_IdealThermoEOS(xDew,&th0g);
        thSys->MixThermoEOS(xDew,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    case FF_SWtype:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataS[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        break;
    default:
        for (j=0;j<numSubs;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataC[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("Tc:%f Pc:%f w:%f k1:%f k2:%f k3:%f\n",dataC[j].Tc,dataC[j].Pc,dataC[j].w,dataC[j].k1,dataC[j].k2,dataC[j].k3);
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        thSys->DewP(&thR.T,c,&dewPguess,&dewP,xDew,substPhiL,substPhiG);
        th0.P=thR.P=th0g.P=thRg.P=dewP;
        option='l';//determine only the liquid volume
        thSys->MixVfromTPeos(&thR.T,&thR.P,xDew,&option,answerL,answerG,&state);
        th0.V=thR.V=answerL[0];
        Z=answerL[2];
        thSys->MixFF_IdealThermoEOS(xDew,&th0);
        thSys->MixThermoEOS(xDew,&thR);
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];

        option='g';//determine only the gas volume
        thSys->MixVfromTPeos(&thRg.T,&thRg.P,c,&option,answerL,answerG,&state);
        th0g.V=thRg.V=answerG[0];
        Zg=answerG[2];
        thSys->MixFF_IdealThermoEOS(c,&th0g);
        thSys->MixThermoEOS(c,&thRg);
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        break;
    }


    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thR.T,thR.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);

    ui->twMixCalc->item(1,1)->setText(QString::number(thRg.P*0.00001));
    ui->twMixCalc->item(2,1)->setText(QString::number(thRg.T-273.15));
    ui->twMixCalc->item(3,1)->setText(QString::number(1));
    //ui->twMixCalc->item(4,1)->setText(QString::number(phiL));
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));
    ui->twMixCalc->item(7,1)->setText(QString::number(thRg.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,1)->setText(QString::number(MW/thRg.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,1)->setText(QString::number(th0g.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,1)->setText(QString::number(th0g.S/MW));
    ui->twMixCalc->item(11,1)->setText(QString::number(th0g.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,1)->setText(QString::number(th0g.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,1)->setText(QString::number(thRg.H/MW));
    ui->twMixCalc->item(14,1)->setText(QString::number(thRg.U/MW));
    ui->twMixCalc->item(15,1)->setText(QString::number(thRg.S/MW));
    ui->twMixCalc->item(16,1)->setText(QString::number(thRg.Cp/MW));
    ui->twMixCalc->item(17,1)->setText(QString::number(thRg.Cv/MW));
    ui->twMixCalc->item(18,1)->setText(QString::number(thRg.SS));
    ui->twMixCalc->item(19,1)->setText(QString::number(thRg.JT*1e5));
    ui->twMixCalc->item(20,1)->setText(QString::number(thRg.IT*1e2));
    ui->twMixCalc->item(21,1)->setText(QString::number(thRg.dP_dT*1e-5));
    ui->twMixCalc->item(22,1)->setText(QString::number(thRg.dP_dV*1e-5));
    ui->twMixCalc->item(29,1)->setText(QString::number(c[0]));
    ui->twMixCalc->item(30,1)->setText(QString::number(c[1]));
    ui->twMixCalc->item(41,1)->setText(QString::number(substPhiG[0]));
    ui->twMixCalc->item(42,1)->setText(QString::number(substPhiG[1]));

    ui->twMixCalc->item(1,2)->setText(QString::number(thR.P*0.00001));
    ui->twMixCalc->item(2,2)->setText(QString::number(thR.T-273.15));
    ui->twMixCalc->item(3,2)->setText(QString::number(0));
    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    //ui->twMixCalc->item(5,2)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
    ui->twMixCalc->item(7,2)->setText(QString::number(thR.V*1e6));//Molar volume cm3/mol
    ui->twMixCalc->item(8,2)->setText(QString::number(MW/thR.V*0.001));//rho kgr/m3
    ui->twMixCalc->item(9,2)->setText(QString::number(th0.H/MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,2)->setText(QString::number(th0.S/MW));
    ui->twMixCalc->item(11,2)->setText(QString::number(th0.Cp/MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,2)->setText(QString::number(th0.Cv/MW));//Cv0 KJ/kgr·K
    ui->twMixCalc->item(13,2)->setText(QString::number(thR.H/MW));
    ui->twMixCalc->item(14,2)->setText(QString::number(thR.U/MW));
    ui->twMixCalc->item(15,2)->setText(QString::number(thR.S/MW));
    ui->twMixCalc->item(16,2)->setText(QString::number(thR.Cp/MW));
    ui->twMixCalc->item(17,2)->setText(QString::number(thR.Cv/MW));
    ui->twMixCalc->item(18,2)->setText(QString::number(thR.SS));
    ui->twMixCalc->item(19,2)->setText(QString::number(thR.JT*1e5));
    ui->twMixCalc->item(20,2)->setText(QString::number(thR.IT*1e2));
    ui->twMixCalc->item(21,2)->setText(QString::number(thR.dP_dT*1e-5));
    ui->twMixCalc->item(22,2)->setText(QString::number(thR.dP_dV*1e-5));
    ui->twMixCalc->item(29,2)->setText(QString::number(xDew[0]));
    ui->twMixCalc->item(30,2)->setText(QString::number(xDew[1]));
    ui->twMixCalc->item(41,2)->setText(QString::number(substPhiL[0]));
    ui->twMixCalc->item(42,2)->setText(QString::number(substPhiL[1]));
}

void FreeFluidsMainWindow::twMixCalcPenvelope(){
    int numPoints=21;
    int i,j;//the loop variable
    double base[numPoints],liquid[numPoints],gas[numPoints],bP[numPoints],dP[numPoints];
    int subsId[2],eosId[2],cp0Id[2];//the ids of the substance, eos selected, only for the defined substances
    QString eosModel[2];//The eos model to use for each defined substance, in text format
    enum FF_EOS eos[2];//The eos model to use for each defined substance, in enum format
    double pintParam[2][2][6];//will hold the interaction parameters between substances
    double T;//the working temperature
    FreeFluidsMainWindow::getMixEosCpSel();//we read the selected Eos and Cp index for each substance
    for (j=0;j<ui->twMixEnvelope->rowCount();j++) for(i=0;i<ui->twMixEnvelope->columnCount();i++) ui->twMixEnvelope->item(j,i)->setText("");//we clear the content of the results table
    //Now we need to read the selections made for: the Id of the substance,the molar fraction, the EOS and the Cp0 formula to use. And the type of EOS
    for (j=0;j<2;j++){
        subsId[j]=ui->twMixComposition->item(j,0)->text().toInt();//Substance Id
        eosId[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Id").toInt();
        eosModel[j]=mixCalcEOSModel[j]->record(eosSel[j]).value("Eos").toString();
        cp0Id[j]=mixCalcCp0Model[j]->record(cp0Sel[j]).value("Id").toInt();
        if (!((subsId[j]>0)&&(eosId[j]>0))) return;//Without substance or eos calculation is impossible
        ConvertEosToEnumeration(&eosModel[j],&eos[j]);//Now it is necessary to put the eos we are using in enumeration format
        //std::cout<<subsId[j]<<" "<<eosId[j]<<" "<<cp0Id[j]<<" "<<eosModel[j].toStdString()<<" "<<eos[j]<<std::endl;
    }
    //we fill all the interaction parameters,from the selection made in screen
    for(int i=0;i<2;i++) for(int j=0;j<2;j++) for(int k=0;k<6;k++) pintParam[i][j][k]=pintParamEos[i][j][k];
    //Now we begin the calculations
    //int corrNum[2];//the cp0 correlation formula to use for each selected subatance
    //double coef[2][13];//The coefficients of the cp0 correlation for each substance
     FF_Correlation cp0[numSubs];
     FF_SaftEOSdata dataP[2];
     FF_SWEOSdata  dataS[2];
     FF_CubicEOSdata dataC[2];
    T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    switch(eos[0]){
    case FF_PCSAFT:
    case FF_DPCSAFT_GV:
    case FF_DPCSAFT_JC:
    case FF_PCSAFTPOL1:
        for (j=0;j<2;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataP[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //std::cout<<eos[j]<<" "<<dataP[j].Tc<<" "<<dataP[j].Pc<<" "<<dataP[j].sigma<<" "<<dataP[j].m<<std::endl;
            //printf("cp0Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
//        FF_PressureEnvelope(eos,&rule,&T,dataP,&pintParam[0][0][0],&numPoints,base,liquid,gas,bP,dP);
        break;
    case FF_SW:
    case FF_IAPWS95:
        for (j=0;j<2;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataS[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        break;
    default:
        for (j=0;j<2;j++){
            GetEosData(&subsId[j],&eos[j],&eosId[j],&cp0Id[j],&db,&dataC[j],&cp0[j]);//We get from the database the eos and cp0 data necessary for calculations
            //printf("Tc:%f Pc:%f w:%f k1:%f k2:%f k3:%f\n",dataC[j].Tc,dataC[j].Pc,dataC[j].w,dataC[j].k1,dataC[j].k2,dataC[j].k3);
            //printf("cp0 Id:%i A:%f B:%f C:%f\n",corrNum[j],coef[j][0],coef[j][1],coef[j][2]);
        }
        //printf("dataC[0] w:%f\n",dataC[0].w);
        //printf("dataC[1] w:%f\n",dataC[1].w);
//        FF_PressureEnvelope(eos,&rule,&T,dataC,&pintParam[0][0][0],&numPoints,base,liquid,gas,bP,dP);
        break;
    }

    for(i=0;i<ui->twMixEnvelope->rowCount()-1;i++){
        ui->twMixEnvelope->item(i,0)->setText(QString::number(base[i]));
        ui->twMixEnvelope->item(i,1)->setText(QString::number(bP[i]));
        ui->twMixEnvelope->item(i,2)->setText(QString::number(liquid[i]));
        ui->twMixEnvelope->item(i,3)->setText(QString::number(dP[i]));
        ui->twMixEnvelope->item(i,4)->setText(QString::number(gas[i]));
    }
    ui->twMixEnvelope->item(i,0)->setText(ui->twMixComposition->item(0,1)->text());
    ui->twMixEnvelope->item(i,1)->setText(QString("P envelope"));
    ui->twMixEnvelope->item(i,2)->setText(QString::number(T));
    ui->twMixEnvelope->item(i,3)->setText(QString("Kelvin"));
}

void FreeFluidsMainWindow::on_actionDisplay_license_triggered()
{
    QDialog *dia = new QDialog(this);
    QFile *file=new QFile("LICENSE",dia);
    QTextEdit *disp= new QTextEdit(dia);
    if (file->open(QFile::ReadOnly | QIODevice::Text)) {
        QTextStream text(file);
        disp->setText(text.readAll());
        disp->resize(400,400);
    }
    else disp->setText("Licensed by C.Trujillo under GPL v.3. Look at the source files for details.");
    disp->showMaximized();
    dia->exec();
    delete dia;
}


