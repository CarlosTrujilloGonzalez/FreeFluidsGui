/*
 * freefluidsmainwindow.cpp
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
    QString driver,database;
    driver="QODBC";//driver to use. here ODBC
    database="Substances.dsn";//In ODBC the database must have been registered with a data source name
    db = QSqlDatabase::addDatabase(driver);
    db.setDatabaseName(database);
    if (!db.open()) {
        QMessageBox::critical(0, "No ODBC connection",
        "It has been impossible to connect to the ODBC database\n"
        "click Cancel to continue with SqLite", QMessageBox::Cancel);
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

    subsData = new FF_SubstanceData;
    subsDataRef= new FF_SubstanceData;
    mix = new FF_MixData;
    //fill with 0 the eos binary interaction parameters array
    for(int i=0;i<15;i++) for(int j=0;j<15;j++) for(int k=0;k<6;k++) mix->intParam[i][j][k]=0;

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
                       <<"I.T.coeff(KJ/bar)"<<"(dP/dT)V(bar/K)"<<"(dP/dV)T(kgr·bar/m3)"<<"(dV/dT)P(m3/kgr·K)"<<"Isobaric expansion coef.(1/K)"
                      <<"Isothermal compression coef.(1/Pa)"<<"Ln of reduced bulk modulus"<<"Vp(bar)"<<"rho liq. sat."<<"rho gas sat."
                     <<"H liq. sat.(KJ/kg)"<<"H gas sat.(KJ/kg)"<<"S liq. sat.(KJ/kg·K)"<<"S gas sat.(KJ/kg·K)"
                     <<"Hv sat(KJ/kgr)"<<"Cp liq.sat(KJ/kgr·K)"<<"Arr liq.saturated (red.res. Helmhotz)"<<" "<< "Arr (reduced residual Helmhotz)"
                      <<"(dArr/dV)T"<<"(d2Arr/dV2)T"<<"(dArr/dT)V"<<"(d2Arr/dT2)V"<<"d2Arr/dTdV"<<"Liq.dens.P corrected (kg/m3)"
                     <<"Liq.dens.Rackett,P corrected"<<"Liq.dens.Tait"<<"Liq.visc.P corrected (Pa·s)"<<"Liq.Th.Cond.(W/m·K)"<<"Liq.Th.Cond. Latini"
                    <<"Surf.Tens.(N/m)"<<"Surf.tens.Sastri"<<"Surf.tens.McLeod"<<"Gas visc.P corrected (Pa·s)"<<"Gas visc.Lucas, P corrected"
                   <<"Gas th.cond.P corrected (W/m·K)"<<"Gas th.cond.Chung, P corrected"<<"Liquid Cp (J/kgr·K)"<<"Liquid Cp. Bondi method"<<"T(K)";
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
    connect(ui->btnSubsToolsFindEOS,SIGNAL(clicked()),this,SLOT(btnSubsToolsFindEos()));

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

    //Button for exporting the substance in Modelica FreeFluids.Media format
    connect(ui->btnSubsToolsExport,SIGNAL(clicked()),this,SLOT(btnSubsToolsExport()));

    //Button for storing the reference substance for corresponding states calculations
    connect(ui->btnSubsToolsKeepRef,SIGNAL(clicked()),this,SLOT(btnSubsToolsKeepRef()));

    //Button for performing corresponding states calculations
    connect(ui->btnSubsToolsDoCScalc,SIGNAL(clicked()),this,SLOT(btnSubsToolsDoCScalc()));

    //Button for storing propane as reference substance for corresponding states calculation
    connect(ui->btnSubsToolsPropaneRef,SIGNAL(clicked()),this,SLOT(btnSubsToolsPropaneRef()));

    //Mixture calculation tab setup
    //*****************************

     mix->numSubs=0;
     //mix->refVpEos=1;
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

    //Combobox for updating the general thermo model
    mix->thModelActEos=1;
    connect(ui->cbMixCalcLiqModelSelec,SIGNAL(currentIndexChanged(int)),this,SLOT(cbMixCalcLiqModelLoad(int)));

    //Combobox for eos type selection
    ui->cbMixCalcEosTypeSelec->setToolTip("Select the type of eos to use");
    ui->cbMixCalcEosTypeSelec->setToolTipDuration(3000);
    connect(ui->cbMixCalcEosTypeSelec,SIGNAL(currentIndexChanged(int)),this,SLOT(cbMixCalcEosTypeLoad(int)));//A new EOS type selection must update available EOS

    //combobox for mixing rule selection
    connect(ui->cbMixCalcMixRuleSelec,SIGNAL(currentIndexChanged(int)),this,SLOT(cbMixCalcMixRuleLoad(int)));//Mixing rule selection is stored

    //Combobox for activity model selection
    ui->cbMixCalcActModelSelec->setToolTip("Select the model to use for activity calculation");
    ui->cbMixCalcActModelSelec->setToolTipDuration(3000);
    connect(ui->cbMixCalcActModelSelec,SIGNAL(currentIndexChanged(int)),this,SLOT(cbMixCalcActModelLoad(int)));//Mixing rule selection is stored

    //combobox for EOS selection, model and view assignation for substances
    for (int i=0;i<15;i++){
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
    ui->cbMixCalcEosSelec7->setModel(mixCalcEOSModel[6]);
    ui->cbMixCalcEosSelec8->setModel(mixCalcEOSModel[7]);
    ui->cbMixCalcEosSelec9->setModel(mixCalcEOSModel[8]);
    ui->cbMixCalcEosSelec10->setModel(mixCalcEOSModel[9]);
    ui->cbMixCalcEosSelec11->setModel(mixCalcEOSModel[10]);
    ui->cbMixCalcEosSelec12->setModel(mixCalcEOSModel[11]);
    ui->cbMixCalcEosSelec13->setModel(mixCalcEOSModel[12]);
    ui->cbMixCalcEosSelec14->setModel(mixCalcEOSModel[13]);
    ui->cbMixCalcEosSelec15->setModel(mixCalcEOSModel[14]);

    //tvMixCalcSelEOS[0] = new QTableView(ui->cbMixCalcEosSelec1 );
    ui->cbMixCalcEosSelec1->setView(tvMixCalcSelEOS[0]);
    ui->cbMixCalcEosSelec2->setView(tvMixCalcSelEOS[1]);
    ui->cbMixCalcEosSelec3->setView(tvMixCalcSelEOS[2]);
    ui->cbMixCalcEosSelec4->setView(tvMixCalcSelEOS[3]);
    ui->cbMixCalcEosSelec5->setView(tvMixCalcSelEOS[4]);
    ui->cbMixCalcEosSelec6->setView(tvMixCalcSelEOS[5]);
    ui->cbMixCalcEosSelec7->setView(tvMixCalcSelEOS[6]);
    ui->cbMixCalcEosSelec8->setView(tvMixCalcSelEOS[7]);
    ui->cbMixCalcEosSelec9->setView(tvMixCalcSelEOS[8]);
    ui->cbMixCalcEosSelec10->setView(tvMixCalcSelEOS[9]);
    ui->cbMixCalcEosSelec11->setView(tvMixCalcSelEOS[10]);
    ui->cbMixCalcEosSelec12->setView(tvMixCalcSelEOS[11]);
    ui->cbMixCalcEosSelec13->setView(tvMixCalcSelEOS[12]);
    ui->cbMixCalcEosSelec14->setView(tvMixCalcSelEOS[13]);
    ui->cbMixCalcEosSelec15->setView(tvMixCalcSelEOS[14]);

    //combobox for Cp0 selection, model and view assignation for substances
    for (int i=0;i<15;i++){
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
    ui->cbMixCalcCp0Selec7->setModel(mixCalcCp0Model[6]);
    ui->cbMixCalcCp0Selec8->setModel(mixCalcCp0Model[7]);
    ui->cbMixCalcCp0Selec9->setModel(mixCalcCp0Model[8]);
    ui->cbMixCalcCp0Selec10->setModel(mixCalcCp0Model[9]);
    ui->cbMixCalcCp0Selec11->setModel(mixCalcCp0Model[10]);
    ui->cbMixCalcCp0Selec12->setModel(mixCalcCp0Model[11]);
    ui->cbMixCalcCp0Selec13->setModel(mixCalcCp0Model[12]);
    ui->cbMixCalcCp0Selec14->setModel(mixCalcCp0Model[13]);
    ui->cbMixCalcCp0Selec15->setModel(mixCalcCp0Model[14]);

    ui->cbMixCalcCp0Selec1->setView(tvMixCalcSelCp0[0]);
    ui->cbMixCalcCp0Selec2->setView(tvMixCalcSelCp0[1]);
    ui->cbMixCalcCp0Selec3->setView(tvMixCalcSelCp0[2]);
    ui->cbMixCalcCp0Selec4->setView(tvMixCalcSelCp0[3]);
    ui->cbMixCalcCp0Selec5->setView(tvMixCalcSelCp0[4]);
    ui->cbMixCalcCp0Selec6->setView(tvMixCalcSelCp0[5]);
    ui->cbMixCalcCp0Selec7->setView(tvMixCalcSelCp0[6]);
    ui->cbMixCalcCp0Selec8->setView(tvMixCalcSelCp0[7]);
    ui->cbMixCalcCp0Selec9->setView(tvMixCalcSelCp0[8]);
    ui->cbMixCalcCp0Selec10->setView(tvMixCalcSelCp0[9]);
    ui->cbMixCalcCp0Selec11->setView(tvMixCalcSelCp0[10]);
    ui->cbMixCalcCp0Selec12->setView(tvMixCalcSelCp0[11]);
    ui->cbMixCalcCp0Selec13->setView(tvMixCalcSelCp0[12]);
    ui->cbMixCalcCp0Selec14->setView(tvMixCalcSelCp0[13]);
    ui->cbMixCalcCp0Selec15->setView(tvMixCalcSelCp0[14]);

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
    ui->twMixIntParamEos->setColumnWidth(7,245);
    ui->twMixIntParamEos->setHorizontalHeaderLabels(QStringList()<<"k1/A"<<"k2/B"<<"k3/C"<<"l1/D"<<"l2/E"<<"l3/F"<<"Eq."<<"Description");
    ui->twMixIntParamEos->setVerticalHeaderLabels(QStringList()<<"i,j"<<"j,i");
    for (int i=0;i<ui->twMixIntParamEos->columnCount();i++){
        for (int j=0;j<ui->twMixIntParamEos->rowCount();j++){
            ui->twMixIntParamEos->setItem(j,i,new QTableWidgetItem());
        }
    }
    connect(ui->twMixIntParamEos,SIGNAL(cellChanged(int,int)),this,SLOT(twMixIntParamEosUpdate(int,int)));

    //button for interaction parameter clear
    connect(ui->btnMixCalcClearIntParam,SIGNAL(clicked()),this,SLOT(twMixIntParamClear()));

    //button for system creation
    connect(ui->btnMixCalcCreateSystem,SIGNAL(clicked()),this,SLOT(btnMixCalcCreateSys()));

    //button for mixture exportation
    connect(ui->btnMixCalcExportMix,SIGNAL(clicked()),this,SLOT(btnMixCalcExportMix()));

    //button for mixture import
    connect(ui->btnMixCalcImportMix,SIGNAL(clicked()),this,SLOT(btnMixCalcImportMix()));

    //Button for stability check of a given composition
     connect(ui->btnMixCalcStabCheck,SIGNAL(clicked()),this,SLOT(mixCalcStabCheck()));

    //Button for mixture bubble T calculation at given P
          connect(ui->btnMixCalcBubbleT,SIGNAL(clicked()),this,SLOT(twMixCalcBubbleT()));

    //Button for mixture dew T calculation at given P
          connect(ui->btnMixCalcDewT,SIGNAL(clicked()),this,SLOT(twMixCalcDewT()));

    //Button for pressure envelope at given T
          connect(ui->btnMixCalcTenvelope,SIGNAL(clicked()),this,SLOT(twMixCalcTenvelope()));

    //Button for mixture bubble P calculation at given T
          connect(ui->btnMixCalcBubbleP,SIGNAL(clicked()),this,SLOT(twMixCalcBubbleP()));

    //Button for mixture dew P calculation at given T
          connect(ui->btnMixCalcDewP,SIGNAL(clicked()),this,SLOT(twMixCalcDewP()));

    //Button for pressure envelope at given T
          connect(ui->btnMixCalcPenvelope,SIGNAL(clicked()),this,SLOT(twMixCalcPenvelope()));

    //Button for mixture VL flash PT
          connect(ui->btnMixCalc2PhPTflash,SIGNAL(clicked()),this,SLOT(twMixCalc2PhFlashPT()));

    //Button for mixture VL flash PT
          connect(ui->btnMixCalc3PhPTflash,SIGNAL(clicked()),this,SLOT(twMixCalc3PhFlashPT()));

    //Button for stability check of resulting phases
          connect(ui->btnMixResStabCheck,SIGNAL(clicked()),this,SLOT(mixResStabCheck()));

   //Button for transport properties calculation
          connect(ui->btnMixResCalcTransport,SIGNAL(clicked()),this,SLOT(mixResCalcTransport()));


    //Mixture results tab setup
    //*************************

    //Table widget for calculated mixture data display
    QStringList mixCalcHorLabels;
    QStringList mixCalcVerLabels;
    mixCalcVerLabels <<"MW"<<"P (bara)"<<"T(K)"<<"Phase fraction"<<"phi liq."<<"phi gas"<<"Z"<<"V(cm3/mol)"<<"rho(kgr/m3)"<<"H0(KJ/kgr)" <<"S0(KJ/kgr·K)"
                     <<"Cp0(KJ/kgr·K)"<<"Cv0(KJ/kgr·K)"<<"H(KJ/kgr)"<<"U(KJ/kgr)"<<"S(KJ/kgr·K)"<<"Cp(KJ/kgr·K)"<<"Cv(KJ/kgr·K)"<<"Sound S.(m/s)"<<"J.T.coeff(K/bar)"
                     <<"I.T.coeff(KJ/bar)"<<"(dP/dT)V(bar/K)"<<"(dP/dV)T(bar/m3)"<< "Arr"<<"(dArr/dV)T"<<"(d2Arr/dV2)T"<<"(dArr/dT)V"<<"(d2Arr/dT2)V"
                     <<"d2Arr/dTdV"<<"Mol fract.1"<<"Mol fract.2"<<"Mol fract.3"<<"Mol fract.4"<<"Mol fract.5"<<"Mol fract.6"<<"Mol fract.7"<<"Mol fract.8"
                    <<"Mol fract.9"<<"Mol fract.10"<<"Mol fract.11"<<"Mol fract.12"<<"Mol fract.13"<<"Mol fract.14"<<"Mol fract.15"<<"Mass fract.1"<<"Mass fract.2"
                     <<"Mass fract.3"<<"Mass fract.4"<<"Mass fract.5"<<"Mass fract.6"<<"Mass fract.7"<<"Mass fract.8"<<"Mass fract.9"<<"Mass fract.10"
                    <<"Mass fract.11"<<"Mass fract.12"<<"Mass fract.13"<<"Mass fract.14"<<"Mass fract.15"<<"Phi 1"<<"Phi 2"<<"Phi 3"<<"Phi 4"<<"Phi 5"<<"Phi 6"
                   <<"Phi 7"<<"Phi 8"<<"Phi 9"<<"Phi 10"<<"Phi 11"<<"Phi 12"<<"Phi 13"<<"Phi 14"<<"Phi 15"<<"gE(reduced)"<<"hE(reduced)"<<"TRANSPORT PROPERTIES"
                  <<"Viscosity(Pa·s)"<<"Thermal cond. (W/m·K)"<<"Surf.tension (N/m)";
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
    delete subsData;
    delete subsDataRef;
    //delete[] substance;
    //delete[] subsPoint;
    delete mix;
    QString database=db.connectionName();
    db.close();
    //db.removeDatabase(database);
}

//Common functions
//****************

//Slot for loading the existing EOS and correlations for the selected substance
void FreeFluidsMainWindow::cbSubsCalcSelLoad(int position)
{
    //Delete old substance,create a new one, and clear screen
    delete subsData;
    subsData= new FF_SubstanceData;
    subsData->id=subsListModel->record(position).value("Id").toInt();
    subsData->refT=0.0;
    subsData->refP=101325;
    subsData->vpCorr.form=0;
    subsData->btCorr.form=0;
    subsData->cp0Corr.form=0;
    subsData->gDensCorr.form=0;
    subsData->gThCCorr.form=0;
    subsData->gViscCorr.form=0;
    subsData->hVsatCorr.form=0;
    subsData->lCpCorr.form=0;
    subsData->lTfromHCorr.form=0;
    subsData->lDensCorr.form=0;
    subsData->lSurfTCorr.form=0;
    subsData->lThCCorr.form=0;
    subsData->lViscCorr.form=0;
    subsData->lBulkModRCorr.form=0;
    subsData->sCpCorr.form=0;
    subsData->sDensCorr.form=0;
    subsData->model=FF_NoType;
    subsData->cubicData.eos=FF_IdealGas;
    subsData->saftData.eos=FF_IdealGas;
    subsData->swData.eos=FF_IdealGas;
    ui->leSubsCalcCp0->setText("");
    ui->leSubsCalcCubic->setText("");
    ui->leSubsCalcSaft->setText("");
    ui->leSubsCalcSW->setText("");
    ui->chbSubsCalcCp0->setChecked(false);
    ui->chbSubsCalcCubic->setChecked(false);
    ui->chbSubsCalcSaft->setChecked(false);
    ui->chbSubsCalcSW->setChecked(false);

    ui->leSubsToolsCp0Corr->setText("");
    ui->leSubsToolsLdensCorr->setText("");
    ui->leSubsToolsVpCorr->setText("");
    ui->leSubsToolsBtCorr->setText("");
    ui->leSubsToolsHvCorr->setText("");
    ui->leSubsToolsLCpCorr->setText("");
    ui->leSubsToolsLTfromHCorr->setText("");
    ui->leSubsToolsLviscCorr->setText("");
    ui->leSubsToolsLthCCorr->setText("");
    ui->leSubsToolsLsurfTCorr->setText("");
    ui->leSubsToolsLbulkModRCorr->setText("");
    ui->leSubsToolsGdensCorr->setText("");
    ui->leSubsToolsGviscCorr->setText("");
    ui->leSubsToolsGthCCorr->setText("");
    ui->leSubsToolsSdensCorr->setText("");
    ui->leSubsToolsSCpCorr->setText("");

    ui->chbSubsToolsCp0->setChecked(false);
    ui->chbSubsToolsLdens->setChecked(false);
    ui->chbSubsToolsVp->setChecked(false);
    ui->chbSubsToolsBt->setChecked(false);
    ui->chbSubsToolsHv->setChecked(false);
    ui->chbSubsToolsLCp->setChecked(false);
    ui->chbSubsToolsLTfromH->setChecked(false);
    ui->chbSubsToolsLvisc->setChecked(false);
    ui->chbSubsToolsLthC->setChecked(false);
    ui->chbSubsToolsLsurfT->setChecked(false);
    ui->chbSubsToolsLbulkModR->setChecked(false);
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
    ui->leSubsToolsMW->setText("0");
    ui->leSubsToolsTc->setText("0");
    ui->leSubsToolsPc->setText("0");
    ui->leSubsToolsRc->setText("0");
    ui->leSubsToolsZc->setText("0");
    ui->leSubsToolsW->setText("0");
    ui->leSubsToolsVdWV->setText("0");
    ui->leSubsToolsMu->setText("0");
    ui->leSubsToolsQ->setText("0");

    GetBasicData(subsData->id,subsData,&db);
    //printf("Tc:%f\n",subsData->baseProp.Tc);


    if (subsData->baseProp.MW > 0) ui->leSubsToolsMW->setText(QString::number(subsData->baseProp.MW));//and now from substance to screen
    else if (subsData->baseProp.MWmono > 0) ui->leSubsToolsMW->setText(QString::number(subsData->baseProp.MWmono));
    if (subsData->baseProp.Tc>0) ui->leSubsToolsTc->setText(QString::number(subsData->baseProp.Tc));
    if (subsData->baseProp.Pc > 0) ui->leSubsToolsPc->setText(QString::number(subsData->baseProp.Pc));
    if ((subsData->baseProp.Vc > 0)&&(subsData->baseProp.MW > 0)) ui->leSubsToolsRc->setText(QString::number(subsData->baseProp.MW/subsData->baseProp.Vc*1e-3));
    else if ((subsData->baseProp.Zc > 0)&&(subsData->baseProp.MW > 0)) ui->leSubsToolsRc->setText(QString::number(subsData->baseProp.MW*subsData->baseProp.Pc/
            (R*subsData->baseProp.Zc*subsData->baseProp.Tc*1e3)));
    if (subsData->baseProp.Zc > 0) ui->leSubsToolsZc->setText(QString::number(subsData->baseProp.Zc));
    if (subsData->baseProp.w > 0) ui->leSubsToolsW->setText(QString::number(subsData->baseProp.w));
    if (subsData->baseProp.VdWV>0) ui->leSubsToolsVdWV->setText(QString::number(subsData->baseProp.VdWV));
    if (subsData->baseProp.mu>0) ui->leSubsToolsMu->setText(QString::number(subsData->baseProp.mu));
    if (subsData->baseProp.Q>0) ui->leSubsToolsQ->setText(QString::number(subsData->baseProp.Q));
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
    queryEos.addBindValue(subsData->id);
    queryEos.exec();
    subsCalcEOSModel->setQuery(queryEos);
    tvSubsCalcSelEOS->setColumnHidden(2,true);
    tvSubsCalcSelEOS->setColumnWidth(0,78);
    tvSubsCalcSelEOS->setColumnWidth(1,220);
    tvSubsCalcSelEOS->setColumnWidth(3,46);
    tvSubsCalcSelEOS->setColumnWidth(4,46);
    tvSubsCalcSelEOS->setColumnWidth(5,72);
    //tvSubsCalcSelEOS->resizeColumnsToContents();
    queryCp0.prepare("SELECT CorrelationEquations.Equation AS Equation, CorrelationParam.Reference AS Reference, CorrelationParam.Tmin AS Tmin, "
                     "CorrelationParam.Tmax AS Tmax, CorrelationParam.Correct AS OK, CorrelationParam.Preferred AS Best, CorrelationParam.Id AS Id FROM PhysProp "
                     "INNER JOIN (CorrelationEquations INNER JOIN (Correlations INNER JOIN CorrelationParam ON "
                     "Correlations.Number = CorrelationParam.NumCorrelation) ON CorrelationEquations.Id = "
                     "Correlations.IdEquation) ON PhysProp.Id = Correlations.IdPhysProp WHERE (((CorrelationParam.IdProduct)=?) "
                     "AND ((PhysProp.Property)=?)) ORDER BY Equation");

    queryCp0.addBindValue(subsListModel->record(position).value("Id"));
    queryCp0.addBindValue("Cp0");
    queryCp0.exec();
    subsCalcCp0Model->setQuery(queryCp0);
    tvSubsCalcSelCp0->setColumnHidden(6,true);
    tvSubsCalcSelCp0->resizeColumnsToContents();
    //ui->lineEdit->setText(subsListModel->record(position).value("Name").toString());
    //Query for loading available correlation parameters for the selected substance
    queryCorr.prepare("SELECT PhysProp.Property AS Property, CorrelationEquations.Equation AS Equation, CorrelationParam.Reference AS Reference, "
                      "CorrelationParam.Tmin AS Tmin, CorrelationParam.Tmax AS Tmax, CorrelationParam.Correct AS OK, CorrelationParam.Preferred AS Best, CorrelationParam.Id AS Id "
                      "FROM PhysProp INNER JOIN (CorrelationEquations INNER JOIN (Correlations INNER JOIN CorrelationParam ON "
                      "Correlations.Number = CorrelationParam.NumCorrelation) ON CorrelationEquations.Id = Correlations.IdEquation) "
                      "ON PhysProp.Id = Correlations.IdPhysProp WHERE (((CorrelationParam.IdProduct)=?))ORDER BY Property;");

    queryCorr.addBindValue(subsListModel->record(position).value("Id"));
    queryCorr.exec();
    subsToolsCorrModel->setQuery(queryCorr);
    tvSubsToolsSelCorr->setColumnHidden(7,true);
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
        subsData->model=FF_CubicType;
        subsData->cubicData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        ui->leSubsCalcCubic->setText(QString::fromStdString(eosInfo));
        ui->chbSubsCalcCubic->setChecked(true);
    }
    else if (eosTypeString=="SAFT"){
        subsData->model=FF_SAFTtype;
        subsData->saftData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        ui->leSubsCalcSaft->setText(QString::fromStdString(eosInfo));
        ui->chbSubsCalcSaft->setChecked(true);
    }
    else if (eosTypeString=="Multiparameter"){
        subsData->model=FF_SWtype;
        subsData->swData.id=subsCalcEOSModel->record(position).value("Id").toInt();
        ui->leSubsCalcSW->setText(QString::fromStdString(eosInfo));
        ui->chbSubsCalcSW->setChecked(true);
    }
    GetEOSData(&subsData->model,subsData,&db);
    //std::cout<<"Cubic eos:"<<subs->cubicData.id<<" Saft eos:"<<subs->saftData.id<<" SW eos:"<<subs->swData.id<<std::endl;
    //printf("Cubic k1:%f\n",subsData->cubicData.k1);
    //printf("SW n[0] :%f\n",subsData->swData.n[0]);
}


//***************************
//===========================

//Slot for cp0 data update in the substance
void FreeFluidsMainWindow::cbSubsCalcCp0Update(int position){

    subsData->cp0Corr.id=subsCalcCp0Model->record(position).value("Id").toInt();
    GetCorrDataById(&subsData->cp0Corr,&db);
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
    double Vp,Tminus,VpMinus,dVp_dT;
    double Hv;//vaporization enthalpy
    double lCpSat;//cp of saturated liquid
    double lCsigma;//specific heat capacity of liquid along saturation line
    double lHsat,gHsat,lSsat,gSsat;//saturated enthalpies and entropies
    double ArrLsat, ZLsat;//Saturated liquid reduced residual Helmholtz
    double ArrDerivatives[6];
    FF_CubicParam param;

    //Now we add some phys.prop. predictions and pressure corrections
    int nPoints=1;
    double lplDens,lDens,lplVisc,lVisc,lThCond,ldgVisc,lpgVisc,gVisc,ldgThC=0,gThC,surfTens,lCp;
    lpgVisc=0;
    ldgVisc=0;

    if(subsData->model==FF_SAFTtype) MW=subsData->saftData.MW;
    else if(subsData->model==FF_SWtype) MW=subsData->swData.MW;
    else MW=subsData->cubicData.MW;
    //FF_TbEOSs(&thR.P,subsData,&Tb);//boiling point calculation
    th0.MW=thR.MW=MW;
    ui->leSubsCalcMW->setText(QString::number(MW));
    ui->leSubsCalcTb->setText(QString::number(Tb-273.15));
    for (i=0;i<11;i++) {
        th0.T=thVp.T=thR.T=initT+i*Tincrement;
        thR.P=1e5*ui->leSubsCalcPres->text().toDouble();//we read the selected pressure. Necessary to do each time, because it is changed

        FF_VfromTPeosS(&thR.T,&thR.P,subsData,&option,answerL,answerG,&state);//Volume, Arr, Z and fugacity coeff. retrieval
        phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
        phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
        if (state=='f') phase="Calc.fail";
        else if ((state=='U')||(state=='l')||(state=='g')) phase="Unique";
        else if (state=='L') phase="Liquid";
        else if (state=='G') phase="Gas";
        else if (state=='E') phase="Equilibrium";
        if ((state=='l')||(state=='L')||(state=='U')||(state=='E')){
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
        else FF_IdealThermoEOS(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&subsData->refT,&subsData->refP,&th0);
        FF_ThermoEOSs(subsData,&thR);
        if (ui->chbSubsCalcSatProp->isChecked()){
            FF_VpEOSs(&thR.T,subsData,&Vp);//Vapor pressure calculation
            Tminus=thR.T-0.01;
            FF_VpEOSs(&Tminus,subsData,&VpMinus);
            dVp_dT=(Vp-VpMinus)/(thR.T-Tminus);
        }
        else{
            Vp=0;
            dVp_dT=0;
        }
        if(subsData->model==FF_SAFTtype) FF_ArrDerSAFT(&thR.T,&thR.V,&subsData->saftData,ArrDerivatives);
        else if(subsData->model==FF_SWtype) FF_ArrDerSWTV(&thR.T,&thR.V,&subsData->swData,ArrDerivatives);
        else{
            FF_FixedParamCubic(&subsData->cubicData,&param);
            FF_ThetaDerivCubic(&thR.T,&subsData->cubicData,&param);
            FF_ArrDerCubic(&thR.T,&thR.V,&param,ArrDerivatives);
        }

        if ((Vp>0) && (Vp<1e10)){//If Vp has been calculated as is lower than Pc

            FF_VfromTPeosS(&thR.T,&Vp,subsData,&option,answerLVp,answerGVp,&state);//We calculate liquid and gas volumes at Vp
            thVp.T=thR.T;
            thVp.V=answerGVp[0];
            FF_ExtResidualThermoEOSs(subsData,&thVp);//with the gas volume we calculate the residual thermo properties
            gHsat=thVp.H+th0.H;
            gSsat=thVp.S+th0.S-R*log(Vp/ thR.P);
            Hv=thVp.H;

            thVp.V=answerLVp[0];
            FF_ExtResidualThermoEOSs(subsData,&thVp);//with the liquid volume we calculate the residual thermo properties
            lHsat=thVp.H+th0.H;
            lSsat=thVp.S+th0.S-R*log(Vp/ thR.P);
            Hv=Hv-thVp.H;//vaporization enthalpy is the difference
            lCpSat=thVp.Cp+th0.Cp;
            ArrLsat=thVp.A/(R*thVp.T);
            ZLsat=thVp.P*thVp.V/(R*thVp.T);
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
        ui->twSubsCalc->item(19,i)->setText(QString::number(thR.dP_dV*1e-5*subsData->baseProp.MW/1000));
        ui->twSubsCalc->item(20,i)->setText(QString::number(-thR.dP_dT/thR.dP_dV*1000/subsData->baseProp.MW));//(dV/dT)P
        ui->twSubsCalc->item(21,i)->setText(QString::number(-thR.dP_dT/(thR.dP_dV*thR.V)));
        ui->twSubsCalc->item(22,i)->setText(QString::number(-1/(thR.dP_dV*thR.V)));//Isothermal compressibility
        ui->twSubsCalc->item(23,i)->setText(QString::number(log(-(thR.V*thR.dP_dV*thR.V)/(8.3144*thR.T))));//Ln of reduced bulk modulus
        if (Vp>0){
            ui->twSubsCalc->item(24,i)->setText(QString::number(Vp/100000));//Vapor pressure bar
            ui->twSubsCalc->item(25,i)->setText(QString::number(MW/answerLVp[0]/1000));//liquid rho at Vp kgr/m3
            ui->twSubsCalc->item(26,i)->setText(QString::number(MW/answerGVp[0]/1000));//gas rho at Vp kgr/m3
            ui->twSubsCalc->item(27,i)->setText(QString::number(lHsat/MW));//sat.liquid enthalpy (KJ/kg)
            ui->twSubsCalc->item(28,i)->setText(QString::number(gHsat/MW));//sat.gas enthalpy (KJ/kg)
            ui->twSubsCalc->item(29,i)->setText(QString::number(lSsat/MW));//sat.liquid entropy (KJ/kg·K)
            ui->twSubsCalc->item(30,i)->setText(QString::number(gSsat/MW));//sat.gas entropy (KJ/kg·K)
            ui->twSubsCalc->item(31,i)->setText(QString::number(Hv/MW));//Saturated vaporization enthalpy
            ui->twSubsCalc->item(32,i)->setText(QString::number(lCpSat/MW));//Saturated liquid heat capacity
            ui->twSubsCalc->item(33,i)->setText(QString::number(ArrLsat));//Saturated liquid reduced residual Helmholtz
            ui->twSubsCalc->item(34,i)->setText(QString::number((lCpSat+dVp_dT*(answerLVp[0]+thR.T*thVp.dP_dT/thVp.dP_dV))/MW));//liquid heat capacity along saturationline
        }
        ui->twSubsCalc->item(35,i)->setText(QString::number(ArrDerivatives[0]));
        ui->twSubsCalc->item(36,i)->setText(QString::number(ArrDerivatives[1]));
        ui->twSubsCalc->item(37,i)->setText(QString::number(ArrDerivatives[2]));
        ui->twSubsCalc->item(38,i)->setText(QString::number(ArrDerivatives[3]));
        ui->twSubsCalc->item(39,i)->setText(QString::number(ArrDerivatives[4]));
        ui->twSubsCalc->item(40,i)->setText(QString::number(ArrDerivatives[5]));

        //Correlations data
        if(subsData->lDensCorr.form>0){
            FF_PhysPropCorr(&subsData->lDensCorr.form,subsData->lDensCorr.coef,&subsData->baseProp.MW,&nPoints,&thR.T,&lplDens);
            if(thR.P>Vp) FF_LiqDensChuehPrausnitz(&subsData->baseProp,&thR.T,&thR.P,&Vp,&lplDens,&lDens);
            else lDens=lplDens;
            ui->twSubsCalc->item(41,i)->setText(QString::number(lDens));
        }
        FF_LiqDensSatRackett(&subsData->baseProp,&subsData->lDens.x,&subsData->lDens.y,&thR.T,&lplDens);
        if(thR.P>Vp) FF_LiqDensChuehPrausnitz(&subsData->baseProp,&thR.T,&thR.P,&Vp,&lplDens,&lDens);
        else lDens=lplDens;
        ui->twSubsCalc->item(42,i)->setText(QString::number(lDens));
        //Tait density calculation is missing here
        if(subsData->lViscCorr.form>0){
            FF_PhysPropCorr(&subsData->lViscCorr.form,subsData->lViscCorr.coef,&subsData->baseProp.MW,&nPoints,&thR.T,&lplVisc);
            FF_LiqViscPcorLucas(&thR.T,&thR.P,&Vp,&subsData->baseProp,&lplVisc,&lVisc);
            ui->twSubsCalc->item(44,i)->setText(QString::number(lVisc));
        }

        if(subsData->lThCCorr.form>0){
            FF_PhysPropCorr(&subsData->lThCCorr.form,subsData->lThCCorr.coef,&subsData->baseProp.MW,&nPoints,&thR.T,&lThCond);
            ui->twSubsCalc->item(45,i)->setText(QString::number(lThCond));
        }
        FF_LiquidThCondLatini(&thR.T,&subsData->baseProp,&lThCond);
        ui->twSubsCalc->item(46,i)->setText(QString::number(lThCond));

        if(subsData->lSurfTCorr.form>0){
            FF_PhysPropCorr(&subsData->lSurfTCorr.form,subsData->lSurfTCorr.coef,&subsData->baseProp.MW,&nPoints,&thR.T,&surfTens);
            ui->twSubsCalc->item(47,i)->setText(QString::number(surfTens));
        }
        FF_SurfTensSastri(&thR.T,&subsData->baseProp,&surfTens);
        ui->twSubsCalc->item(48,i)->setText(QString::number(surfTens));
        FF_SurfTensMcLeod(&thR.T,subsData,&surfTens);
        ui->twSubsCalc->item(49,i)->setText(QString::number(surfTens));


        if(subsData->gViscCorr.form>0){
            FF_PhysPropCorr(&subsData->gViscCorr.form,subsData->gViscCorr.coef,&subsData->baseProp.MW,&nPoints,&thR.T,&lpgVisc);
            FF_GasViscTPcpLucas(&thR.T,&thR.P,&subsData->baseProp,&lpgVisc,&gVisc);
            //FF_GasViscTVcpChung(&thR.T,&thR.V,&subsData->baseProp,&lpgVisc,&gVisc);
            ui->twSubsCalc->item(50,i)->setText(QString::number(gVisc));
        }

        lpgVisc=0;
        ldgVisc=0;
        //FF_GasViscTVcpChung(&thR.T,&thR.V,&subsData->baseProp,&ldgVisc,&gVisc);
        FF_GasViscTPcpLucas(&thR.T,&thR.P,&subsData->baseProp,&lpgVisc,&gVisc);
        ui->twSubsCalc->item(51,i)->setText(QString::number(gVisc));

        if(subsData->gThCCorr.form>0){
            FF_PhysPropCorr(&subsData->gThCCorr.form,subsData->gThCCorr.coef,&subsData->baseProp.MW,&nPoints,&thR.T,&ldgThC);
            FF_GasThCondTVcorChung(&thR.T,&thR.V,&subsData->baseProp,&ldgThC,&gThC);
            ui->twSubsCalc->item(52,i)->setText(QString::number(gThC));
        }
        double CpSI=th0.Cp*1000/MW;
        FF_GasLpThCondTCpChung(&thR.T,&CpSI,&subsData->baseProp,&ldgThC);
        FF_GasThCondTVcorChung(&thR.T,&thR.V,&subsData->baseProp,&ldgThC,&gThC);
        ui->twSubsCalc->item(53,i)->setText(QString::number(gThC));

        if(subsData->lCpCorr.form>0){
            FF_PhysPropCorr(&subsData->lCpCorr.form,subsData->lCpCorr.coef,&subsData->baseProp.MW,&nPoints,&thR.T,&lCp);
            ui->twSubsCalc->item(54,i)->setText(QString::number(lCp));
        }
        FF_LiqCpBondi(subsData,&thR.T,&lCp);
        ui->twSubsCalc->item(55,i)->setText(QString::number(lCp));
        ui->twSubsCalc->item(56,i)->setText(QString::number(thR.T));
    }


    QString type;
    int nColumns=ui->twSubsCalc->columnCount();
    double x[nColumns];
    for (i=0;i<11;i++) {
        x[i]=initT+i*Tincrement;
    }
}

//Slot for alternative calculation (not from T and P)
void FreeFluidsMainWindow::btnSubsCalcAltCalc(){


    //And now we make calculations
    FF_ThermoProperties thR;//here will be stored the result of the calculations
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    double MW;
    double liqFraction;
    char var;

    //First we need to get the eos data
    if (subsData->model==FF_SAFTtype) MW=subsData->saftData.MW;
    else if (subsData->model==FF_SWtype) MW=subsData->swData.MW;
    else MW=subsData->cubicData.MW;


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
        if (subsData->model==FF_SAFTtype) FF_ThermoEOSfromPX(&subsData->model,&subsData->saftData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else if (subsData->model==FF_SWtype) FF_ThermoEOSfromPX(&subsData->model,&subsData->swData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else FF_ThermoEOSfromPX(&subsData->model,&subsData->cubicData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        break;
    case 3:
    case 4:
        if (subsData->model==FF_SAFTtype) FF_ThermoEOSfromVX(&subsData->model,&subsData->saftData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else if (subsData->model==FF_SWtype) FF_ThermoEOSfromVX(&subsData->model,&subsData->swData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
        else FF_ThermoEOSfromVX(&subsData->model,&subsData->cubicData,&subsData->cp0Corr.form,subsData->cp0Corr.coef,&refT,&refP,&var,&thR,&liqFraction);
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
    QString subsName = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("Substance name (Compatible with Modelica):"));
    QString subsDescription = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("Description for better identification:"));

    QFileDialog *dia = new QFileDialog(this,"Choose directory and file name without extension");
    //dia->setNameFilter("*.mo");
    dia->showNormal();
    QString fileName,fileBinary,fileText;
    if (dia->exec())
        fileName = dia->selectedFiles().first();
    std::cout<<fileName.toStdString()<<std::endl;
    fileBinary=fileName+".sd";
    fileText=fileName+".txt";

    QFile *file=new QFile(fileText,this);
    if (file->open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(file);
        out.setRealNumberNotation(QTextStream::ScientificNotation);
        out<<"//To be copied in the FreeFluids.ExternalMedia.Fluids package. Later you can delete this file\n";
        out<<"  package "<<subsName.toUtf8()<<"\n    extends ExternalTPMedium(final mediumName = \""<<subsName.toUtf8()<<"\",\n";
        out<<" fluidK(casRegistryNumber = \""<<subsData->CAS<<"\", description = \""<<subsDescription.toUtf8()<<"\", molarMass = "<<subsData->baseProp.MW<<"),\n";
        out<<" final onePhase=false, thermoModel=1, refState=2);\n  end "<<subsName.toUtf8()<<";";
    }
    file->close();
    delete file;


    FILE *outfile;
    // open file for writing
    outfile = fopen (fileBinary.toStdString().c_str(),"wb");
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
    int i,n=4;
    for (i=0;i<11;i++)ui->twSubsTools->item(i,0)->setText(QString::number(ui->twSubsCalc->item(0,i)->text().toDouble()+273.15));//We fill the temperatures
    if (ui->chbSubsCalcTransfVp->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(24,i)->text().toDouble()*1e5));
    n=5;
    }
    if (ui->chbSubsCalcTransfSLd->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(ui->twSubsCalc->item(25,i)->text());
    n=5;
    }
    if (ui->chbSubsCalcTransfSGd->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(ui->twSubsCalc->item(26,i)->text());
    n=5;
    }
    if (ui->chbSubsCalcTransfSLCp->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(32,i)->text().toDouble()*1e3));
    n=5;
    }
    if (ui->chbSubsCalcTransfSLH->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(27,i)->text().toDouble()*1e3));
    n=5;
    }
    if (ui->chbSubsCalcTransfSGH->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(28,i)->text().toDouble()*1e3));
    n=5;
    }
    if (ui->chbSubsCalcTransfSLS->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(29,i)->text().toDouble()*1e3));
    n=5;
    }
    if (ui->chbSubsCalcTransfSGS->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(30,i)->text().toDouble()*1e3));
    n=5;
    }
    if (ui->chbSubsCalcTransfHv->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(31,i)->text().toDouble()*1e3));
    n=5;
    }
    if (ui->chbSubsCalcTransfDens->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(ui->twSubsCalc->item(6,i)->text());
    n=5;
    }
    if (ui->chbSubsCalcTransfLnBulkModR->isChecked()==true){
        for (i=0;i<11;i++)ui->twSubsTools->item(i,n)->setText(QString::number(ui->twSubsCalc->item(23,i)->text().toDouble()));
    n=5;
    }
}

//Substance tools tab functions
//*****************************

//Slot for correlation selection load on substance and screen
/*Receives the selected position of the combobox. Retrieves the correlation number. Gets the correlation from the database.
  Adds the data to the correspondig correlation of the substance*/
void FreeFluidsMainWindow::cbSubsToolsCorrLoad(int position){
    QString corrProp;
    FF_Correlation corr;
    corrProp=subsToolsCorrModel->record(position).value("Property").toString();
    corr.id=subsToolsCorrModel->record(ui->cbSubsToolsSelCorr->currentIndex()).value("Id").toInt();
    GetCorrDataById(&corr,&db);

    if (corrProp=="Cp0"){
        subsData->cp0Corr=corr;
        ui->leSubsToolsCp0Corr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        //std::cout<<subs->lDensCorr.coef[0]<<std::endl;
        ui->chbSubsToolsCp0->setChecked(true);
        ui->leSubsCalcCp0->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        ui->chbSubsCalcCp0->setChecked(true);
    }

    else if (corrProp=="Ldens"){
        subsData->lDensCorr=corr;
        ui->leSubsToolsLdensCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        //std::cout<<subs->lDensCorr.coef[0]<<std::endl;
        ui->chbSubsToolsLdens->setChecked(true);
    }
    else if (corrProp=="Vp"){
        subsData->vpCorr=corr;
        ui->leSubsToolsVpCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        ui->chbSubsToolsVp->setChecked(true);
    }
    else if (corrProp=="Bt"){
        //subs->BtCorr=corr;
        subsData->btCorr=corr;
        ui->leSubsToolsBtCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
        //ui->chbSubsToolsBt->setChecked(true);
    }
    else if (corrProp=="HvSat"){
            subsData->hVsatCorr=corr;
            ui->leSubsToolsHvCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsHv->setChecked(true);
    }
    else if (corrProp=="LCp"){
            subsData->lCpCorr=corr;
            ui->leSubsToolsLCpCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLCp->setChecked(true);
    }
    else if (corrProp=="LTfromH"){
            subsData->lTfromHCorr=corr;
            ui->leSubsToolsLTfromHCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
    }
    else if (corrProp=="Lvisc"){
            subsData->lViscCorr=corr;
            ui->leSubsToolsLviscCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLvisc->setChecked(true);
    }
    else if (corrProp=="LthC"){
            subsData->lThCCorr=corr;
            ui->leSubsToolsLthCCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLthC->setChecked(true);
    }
    else if (corrProp=="LsurfT"){
            subsData->lSurfTCorr=corr;
            ui->leSubsToolsLsurfTCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLsurfT->setChecked(true);
    }
    else if (corrProp=="LbulkModR"){
            subsData->lBulkModRCorr=corr;
            ui->leSubsToolsLbulkModRCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsLbulkModR->setChecked(true);
    }
    else if (corrProp=="GdensSat"){
            subsData->gDensCorr=corr;
            ui->leSubsToolsGdensCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsGdens->setChecked(true);
    }
    else if (corrProp=="Gvisc"){
            subsData->gViscCorr=corr;
            ui->leSubsToolsGviscCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsGvisc->setChecked(true);
    }
    else if (corrProp=="GthC"){
            subsData->gThCCorr=corr;
            ui->leSubsToolsGthCCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsGthC->setChecked(true);
    }
    else if (corrProp=="Sdens"){
            subsData->sDensCorr=corr;
            ui->leSubsToolsSdensCorr->setText(subsToolsCorrModel->record(position).value("Reference").toString());
            ui->chbSubsToolsSdens->setChecked(true);
    }
    else if (corrProp=="SCp"){
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
    int j;
    //Now we make calculations
    double minT=ui->leSubsToolsMinTemp->text().toDouble()+273.15;
    double maxT=ui->leSubsToolsMaxTemp->text().toDouble()+273.15;
    int nPoints=ui->leSubsToolsNumPoints->text().toInt();
    double T[nPoints],y[nPoints];
    double Tincrement=(maxT-minT)/(nPoints-1);//temperature increments between calculations
    for (i=0;i<nPoints;i++) T[i]=minT+i*Tincrement;
    int initRow=ui->leSubsToolsInitRow->text().toInt();
    for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,0)->setText(QString::number(T[i-initRow+1]));//We fill the temperatures
    /*
    if (ui->chbSubsToolsCp0->isChecked()==true){
        FF_PhysPropCorr(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Cp0");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the vapor pressure
        n++;
    }
    */
    if (ui->chbSubsToolsCp0->isChecked()==true){
        double H[nPoints],S[nPoints];
        FF_PhysPropCorr(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Cp0");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the vapor pressure
        n++;
        FF_SpecificEnthalpyEntropyCorr(&subsData->cp0Corr.form,subsData->cp0Corr.coef,&subsData->baseProp.MW,&nPoints,T,H,S);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("H0");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(H[i-initRow+1]));//We fill H0
        n++;
        ui->twSubsTools->horizontalHeaderItem(n)->setText("S0");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(S[i-initRow+1]));//We fill S0
        n++;
    }
    if (ui->chbSubsToolsVp->isChecked()==true){
        FF_PhysPropCorr(&subsData->vpCorr.form,subsData->vpCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Vapor press.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the vapor pressure
        n++;
    }
    if (ui->chbSubsToolsLdens->isChecked()==true){
        if (subsData->baseProp.MWmono>0) FF_PhysPropCorr(&subsData->lDensCorr.form,subsData->lDensCorr.coef,&subsData->baseProp.MWmono,&nPoints,T,y);
        else FF_PhysPropCorr(&subsData->lDensCorr.form,subsData->lDensCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);//we call the calculation routine
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.dens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the liquid density
        n++;
    }
    if (ui->chbSubsToolsHv->isChecked()==true){
        FF_PhysPropCorr(&subsData->hVsatCorr.form,subsData->hVsatCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Hv");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the heat of vaporization
        n++;
    }
    if ((ui->chbSubsToolsLCp->isChecked()==true)&&(n<4)){
        double H[nPoints],S[nPoints];
        FF_PhysPropCorr(&subsData->lCpCorr.form,subsData->lCpCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.Cp");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the liquid Cp
        n++;
        FF_SpecificEnthalpyEntropyCorr(&subsData->lCpCorr.form,subsData->lCpCorr.coef,&subsData->baseProp.MW,&nPoints,T,H,S);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.H");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(H[i-initRow+1]));//We fill the liquid H
        n++;
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.S");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(S[i-initRow+1]));//We fill the liquid S
        n++;
    }    
    if (ui->chbSubsToolsLvisc->isChecked()==true){
        FF_PhysPropCorr(&subsData->lViscCorr.form,subsData->lViscCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.visc.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the liquid viscosity
        n++;
    }
    if ((ui->chbSubsToolsLthC->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->lThCCorr.form,subsData->lThCCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.th.cond.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the liquid thermal conductivity
        n++;
    }
    if ((ui->chbSubsToolsLsurfT->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->lSurfTCorr.form,subsData->lSurfTCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.s.tens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the surface tension
        n++;
    }
    if ((ui->chbSubsToolsLbulkModR->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->lBulkModRCorr.form,subsData->lBulkModRCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Liq.bulk mod.R.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the liquid isothermal compressibility
        n++;
    }

    if ((ui->chbSubsToolsGdens->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->gDensCorr.form,subsData->gDensCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Gas dens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the gas saturated density
        n++;
    }
    if ((ui->chbSubsToolsGvisc->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->gViscCorr.form,subsData->gViscCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Gas visc.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the gas viscosity
        n++;
    }
    if ((ui->chbSubsToolsGthC->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->gThCCorr.form,subsData->gThCCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Gas th.con.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the gas thermal conductivity
        n++;
    }
    if ((ui->chbSubsToolsSdens->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->sDensCorr.form,subsData->sDensCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Sol.dens.");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the solid density
        n++;
    }
    if ((ui->chbSubsToolsSCp->isChecked()==true)&&(n<6)){
        FF_PhysPropCorr(&subsData->sCpCorr.form,subsData->sCpCorr.coef,&subsData->baseProp.MW,&nPoints,T,y);
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Sol.Cp");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i-initRow+1]));//We fill the solid heat capacity
    }

    if ((ui->chbSubsToolsVpAmbrose->isChecked()==true)&&(n<6)){
        //std::cout<<T[0]<<std::endl;
        for (j=0;j<nPoints;j++){
            FF_VpAmbroseWalton(&subsData->baseProp,&T[j],&y[j]);
        }
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Vp Ambrose");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the Ambrose vapor pressure
    n++;
    }
    if ((ui->chbSubsToolsVpRiedel->isChecked()==true)&&(n<6)){
        double tRef,pRef;
        tRef=ui->leSubsToolsVpTref->text().toDouble();
        pRef=ui->leSubsToolsVpPref->text().toDouble();
        //std::cout<<T[0]<<std::endl;
        for (j=0;j<nPoints;j++){
            FF_VpRiedelVetere(&subsData->baseProp,&tRef,&pRef,&T[j],&y[j]);
        }
        ui->twSubsTools->horizontalHeaderItem(n)->setText("Vp Riedel");
        for (i=initRow-1;i<(initRow-1+nPoints);i++)ui->twSubsTools->item(i,n)->setText(QString::number(y[i]));//We fill the Ambrose vapor pressure
    n++;
    }
    if ((ui->chbSubsToolsLdensRa->isChecked()==true)&&(n<6)){
        double tRef,dRef;
        tRef=ui->leSubsToolsLdensTref->text().toDouble();
        dRef=ui->leSubsToolsLdensDref->text().toDouble();
        //std::cout<<T[0]<<std::endl;
        for (j=0;j<nPoints;j++){
            FF_LiqDensSatRackett(&subsData->baseProp,&tRef,&dRef,&T[j],&y[j]);
        }
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
    case FF_PPDS15:
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

    ui->leSubsToolsCorrError->setText("");
    ui->leSubsToolsVpError->setText("");
    ui->leSubsToolsLdensError->setText("");
    ui->leSubsToolsZcError->setText("");

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
    case FF_PPDS15:
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
void FreeFluidsMainWindow::btnSubsToolsFindEos(){
    //We clear the results
    ui->leSubsToolsCoef0->setText("");
    ui->leSubsToolsCoef1->setText("");
    ui->leSubsToolsCoef2->setText("");
    ui->leSubsToolsCoef3->setText("");
    ui->leSubsToolsCoef4->setText("");
    ui->leSubsToolsCoef5->setText("");

    unsigned optTime;
    if (ui->spbSubsToolsTime->value()==0) optTime=30;
    else optTime=60*ui->spbSubsToolsTime->value();

    //We need to know where is the data, and the EOS to use
    int fromRow,toRow,eos,numCoef,i,j;
    eos=ui->cbSubsToolsSelOptEOS->currentIndex();
    fromRow=ui->leSubsToolsFromRow->text().toInt();
    toRow=ui->leSubsToolsToRow->text().toInt();
    if(eos<13){//Cubic eos
        FF_CubicFitData data;
        data.eos=&subsData->cubicData;
        data.nPoints=toRow-fromRow+1;
        //We load the eos data
        data.eosType=FF_CubicType;
        data.eos->MW=ui->leSubsToolsMW->text().toDouble();
        data.eos->Tc=ui->leSubsToolsTc->text().toDouble();
        data.eos->Pc=ui->leSubsToolsPc->text().toDouble();
        data.eos->Zc=ui->leSubsToolsZc->text().toDouble();
        data.eos->w=ui->leSubsToolsW->text().toDouble();
        data.eos->c=0;
        data.eos->k1=0;
        data.eos->k2=0;
        data.eos->k3=0;
        data.eos->k4=0;

        data.ldensFilter=ui->spbSubsToolsDens->value() /100.0;
        data.zcFilter=ui->spbSubsToolsZc->value()/100.0;
        data.error=+HUGE_VALF;
        data.vpError=+HUGE_VALF;
        data.ldensError=+HUGE_VALF;
        //printf("%f %f\n",data.ldensFilter,data.zcFilter);

        //We assign the EOS to use, and the number of coefficients to find
        numCoef=3;
        switch (eos){
        case 1:
            data.eos->eos=FF_PR78;
            break;
        case 2:
            data.eos->eos=FF_PRSV1;
            numCoef=1;
            break;
        case 3:
            data.eos->eos=FF_PRMELHEM;
            numCoef=2;
            break;
        case 4:
            data.eos->eos=FF_PRALMEIDA;
            break;
        case 5:
            data.eos->eos=FF_PRSOF;
            numCoef=2;
            break;
        case 6:
            data.eos->eos=FF_PRMC;
            break;
        case 7:
            data.eos->eos=FF_PRTWU91;
            break;
        case 8:
            data.eos->eos=FF_PRFIT4;
            numCoef=4;
            data.eos->c=-1;
            break;
        case 9:
            data.eos->eos=FF_PRvTWU91;
            break;
        case 10:
            data.eos->eos=FF_SRKSOF;
            numCoef=2;
            break;
        case 11:
            data.eos->eos=FF_SRKMC;
            break;
        case 12:
            data.eos->eos=FF_SRKTWU91;
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
        //This is the calculation
        FF_OptCubicParam(optTime,numCoef,lb,ub,enforce,&data,coef,&error);

        //We write the results to the table
        ui->leSubsToolsCoef0->setText(QString::number(coef[0]));
        if (numCoef>1) ui->leSubsToolsCoef1->setText(QString::number(coef[1]));
        if (numCoef>2) ui->leSubsToolsCoef2->setText(QString::number(coef[2]));
        if (numCoef>3) ui->leSubsToolsCoef3->setText(QString::number(coef[3]));
        if (numCoef>4) ui->leSubsToolsCoef4->setText(QString::number(coef[4]));
        if (numCoef>5) ui->leSubsToolsCoef5->setText(QString::number(coef[5]));
        ui->leSubsToolsCorrError->setText(QString::number(error*100));
        ui->leSubsToolsVpError->setText(QString::number(data.vpError*100));
        if ((data.eos->eos==FF_PR78)||(data.eos->eos==FF_PRFIT3)||(data.eos->eos==FF_PRFIT4)) ui->leSubsToolsLdensError->setText(QString::number(data.ldensError*100));
        else ui->leSubsToolsLdensError->setText("");
        ui->leSubsToolsZcError->setText("");

        //Update the coefficients with the best ones
        subsData->model=FF_CubicType;
        switch (subsData->cubicData.eos){
        case FF_PR78:
            subsData->cubicData.Tc=ui->leSubsToolsCoef0->text().toDouble();
            subsData->cubicData.Pc=ui->leSubsToolsCoef1->text().toDouble();
            subsData->cubicData.w=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case FF_PRSV1:
            subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            break;
        case FF_PRMELHEM:
        case FF_PRSOF:
        case FF_SRKSOF:
            subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            break;
        case FF_PRALMEIDA:
        case FF_PRMC:
        case FF_PRTWU91:
        case FF_PRvTWU91:
        case FF_SRKMC:
        case FF_SRKTWU91:
            subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            break;
        case FF_PRFIT4:
            subsData->cubicData.k1=ui->leSubsToolsCoef0->text().toDouble();
            subsData->cubicData.k2=ui->leSubsToolsCoef1->text().toDouble();
            subsData->cubicData.k3=ui->leSubsToolsCoef2->text().toDouble();
            subsData->cubicData.k4=ui->leSubsToolsCoef3->text().toDouble();
            subsData->cubicData.c=-1.0;
            break;
        }
        ui->leSubsCalcCubic->setText("From optimization coefficients");
        ui->chbSubsCalcCubic->setChecked(true);
        ui->leSubsCalcSaft->setText("");
        ui->chbSubsCalcSaft->setChecked(false);
    }
    else{//SAFT eos
        data = new FF_SAFTFitData;
        data->eos=&subsData->saftData;
        data->nPoints=toRow-fromRow+1;
        //We load the eos data
        data->eosType=FF_SAFTtype;
        data->xp=0;
        data->eos->MW=ui->leSubsToolsMW->text().toDouble();
        data->eos->Tc=ui->leSubsToolsTc->text().toDouble();
        data->eos->Pc=ui->leSubsToolsPc->text().toDouble();
        data->eos->Zc=ui->leSubsToolsZc->text().toDouble();
        data->eos->w=ui->leSubsToolsW->text().toDouble();
        data->eos->mu=ui->leSubsToolsMu->text().toDouble();
        data->eos->Q=ui->leSubsToolsQ->text().toDouble();
        data->eos->sigma=0;
        data->eos->m=0;
        data->eos->epsilon=0;
        data->eos->nAcid=0;
        data->eos->nPos=0;
        data->eos->nNeg=0;
        data->eos->kAB=0;
        data->eos->epsilonAB=0;
        data->eos->xp=0;
        data->eos->la=0;
        data->eos->lr=0;
        data->eos->chi=0;

        data->ldensFilter=ui->spbSubsToolsDens->value() /100.0;
        data->zcFilter=ui->spbSubsToolsZc->value()/100.0;
        data->error=+HUGE_VALF;
        data->vpError=+HUGE_VALF;
        data->ldensError=+HUGE_VALF;
        //printf("%f %f\n",data->ldensFilter,data->zcFilter);

        //We assign the EOS to use, and the number of coefficients to find

        numCoef=3;
        switch (eos){
        case 13:
            data->eos->eos=FF_PCSAFT;
            break;
        case 14:
            data->eos->eos=FF_PCSAFT2B;
            numCoef=5;
            data->eos->nPos=1;
            data->eos->nNeg=1;
            break;
        case 15:
            data->eos->eos=FF_PCSAFT3B;
            numCoef=5;
            data->eos->nPos=2;
            data->eos->nNeg=1;
            break;
        case 16:
            data->eos->eos=FF_PCSAFT4C;
            numCoef=5;
            data->eos->nPos=2;
            data->eos->nNeg=2;
            break;
        case 17:
            data->eos->eos=FF_PCSAFT1A;
            numCoef=5;
            data->eos->nAcid=1;
            break;
        case 18:
            data->eos->eos=FF_PPCSAFT_GV;
            numCoef=3;
            data->eos->xp=ui->leSubsToolsNumDipoles->text().toDouble();
            break;
        case 19:
            data->eos->eos=FF_PPCSAFT_JC;
            numCoef=3;
            data->xp=ui->leSubsToolsNumDipoles->text().toDouble();//To be transferred later divided by m
            break;
        case 20:
            data->eos->eos=FF_PPCSAFT2B_GV;
            numCoef=5;
            data->eos->nPos=1;
            data->eos->nNeg=1;
            data->eos->xp=ui->leSubsToolsNumDipoles->text().toDouble();
            break;
        case 21:
            data->eos->eos=FF_PPCSAFT2B_JC;
            numCoef=5;
            data->eos->nPos=1;
            data->eos->nNeg=1;
            data->xp=ui->leSubsToolsNumDipoles->text().toDouble();
            break;
        case 22:
            data->eos->eos=FF_PPCSAFT3B_GV;
            numCoef=5;
            data->eos->nPos=2;
            data->eos->nNeg=1;
            data->eos->xp=ui->leSubsToolsNumDipoles->text().toDouble();
            break;
        case 23:
            data->eos->eos=FF_SAFTVRMie;
            data->eos->la=6;
            numCoef=6;
            break;
        case 24:
            data->eos->eos=FF_SAFTVRMie2B;
            data->eos->la=6;
            numCoef=6;
            data->eos->nPos=1;
            data->eos->nNeg=1;
            break;
        case 25:
            data->eos->eos=FF_PSAFTVRMie_GV;
            data->eos->la=6;
            numCoef=6;
            data->eos->xp=ui->leSubsToolsNumDipoles->text().toDouble();
            break;
        case 26:
            data->eos->eos=FF_PSAFTVRMie_JC;
            data->eos->la=6;
            numCoef=6;
            data->xp=ui->leSubsToolsNumDipoles->text().toDouble();
            break;
        }
        data->nVpPoints=0;
        data->nLdPoints=0;
        for (i=0;i<data->nPoints;i++){//We read the data in the table
            //data->points[i][0]=ui->twSubsTools->item(i+fromRow-1,0)->text().toDouble();
            //data->points[i][1]=ui->twSubsTools->item(i+fromRow-1,1)->text().toDouble();
            //data->points[i][2]=ui->twSubsTools->item(i+fromRow-1,2)->text().toDouble();
            //printf("%f %f %f\n",data->points[i][0],data->points[i][1],data->points[i][2]);
            if(ui->twSubsTools->item(i+fromRow-1,1)->text().toDouble()>0){
                data->vpPoints[data->nVpPoints][0]=ui->twSubsTools->item(i+fromRow-1,0)->text().toDouble();
                data->vpPoints[data->nVpPoints][1]=ui->twSubsTools->item(i+fromRow-1,1)->text().toDouble();
                data->nVpPoints++;
            }
            if(ui->twSubsTools->item(i+fromRow-1,2)->text().toDouble()>0){
                data->ldPoints[data->nLdPoints][0]=ui->twSubsTools->item(i+fromRow-1,0)->text().toDouble();
                data->ldPoints[data->nLdPoints][1]=ui->twSubsTools->item(i+fromRow-1,2)->text().toDouble();
                if(ui->twSubsTools->item(i+fromRow-1,3)->text().toDouble()>0)
                    data->ldPoints[data->nLdPoints][2]=ui->twSubsTools->item(i+fromRow-1,3)->text().toDouble();
                else
                    data->ldPoints[data->nLdPoints][2]=ui->twSubsTools->item(i+fromRow-1,1)->text().toDouble();
                data->nLdPoints++;
            }
        }
        //for(i=0;i<data->nLdPoints;i++) printf("%f %f %f\n",data->ldPoints[i][0],data->ldPoints[i][1],data->ldPoints[i][2]);
        //for(i=0;i<data->nVpPoints;i++) printf("%f %f\n",data->vpPoints[i][0],data->vpPoints[i][1]);
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
        //This is the calculation
        FF_OptSAFTparam(optTime,numCoef,lb,ub,enforce,data,coef,&error);

        //We write the results to the table
        ui->leSubsToolsCoef0->setText(QString::number(coef[0]));
        if (numCoef>1) ui->leSubsToolsCoef1->setText(QString::number(coef[1]));
        if (numCoef>2) ui->leSubsToolsCoef2->setText(QString::number(coef[2]));
        if (numCoef>3) ui->leSubsToolsCoef3->setText(QString::number(coef[3]));
        if (numCoef>4) ui->leSubsToolsCoef4->setText(QString::number(coef[4]));
        if (numCoef>5) ui->leSubsToolsCoef5->setText(QString::number(coef[5]));
        ui->leSubsToolsCorrError->setText(QString::number(error*100));
        ui->leSubsToolsVpError->setText(QString::number(data->vpError*100));
        ui->leSubsToolsLdensError->setText(QString::number(data->ldensError*100));
        ui->leSubsToolsZcError->setText(QString::number(data->zcError*100));

        //Update the coefficients with the best ones
        subsData->model=FF_SAFTtype;
        subsData->saftData.sigma=coef[0];
        subsData->saftData.m=coef[1];
        subsData->saftData.epsilon=coef[2];
        if (numCoef>3) subsData->saftData.kAB=coef[3];
        if (numCoef>4) subsData->saftData.epsilonAB=coef[4];
        if (numCoef>5) subsData->saftData.lr=coef[5];
        if ((subsData->saftData.eos==FF_PPCSAFT_JC)||(subsData->saftData.eos==FF_PPCSAFT2B_JC)||(subsData->saftData.eos==FF_PSAFTVRMie_JC)) subsData->saftData.xp=ui->leSubsToolsNumDipoles->text().toDouble()/coef[1];

        ui->leSubsCalcCubic->setText("");
        ui->chbSubsCalcCubic->setChecked(false);
        ui->leSubsCalcSaft->setText("From optimization coefficients");
        ui->chbSubsCalcSaft->setChecked(true);


        delete data;
    }


}


//Slot for clearing content in the table
void  FreeFluidsMainWindow::twSubsToolsClear(){
    int i,j;//the loop variables
    for (i=0;i<ui->twSubsTools->rowCount();i++) for (j=0;j<ui->twSubsTools->columnCount();j++) ui->twSubsTools->item(i,j)->setText("");//we clear the content
    for (i=0;i<ui->twSubsTools->columnCount();i++) ui->twSubsTools->horizontalHeaderItem(i)->setText("");

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
        AddEosToDataBase(subsData->id,eosType,&subsData->cubicData,&lowLimit,&highLimit,&comment,&db);
    }
    else{
        eosType=FF_SAFTtype;
        //printf("saftData.eos:%i\n",saftData.eos);
        AddEosToDataBase(subsData->id,eosType,&subsData->saftData,&lowLimit,&highLimit,&comment,&db);
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
    case 1://Cp0 DIPPR107
        corr.form=200;
        break;
    case 2://Vp Wagner 2,5
        corr.form=24;
        break;
    case 3://Vp DIPPR101
        corr.form=20;
        break;
    case 4://Boil temp. Polynomia2
        corr.form=130;
        break;
    case 5://Hv DIPPR106
        corr.form=91;
        break;
    case 6://Liquid Cp DIPPR100
        corr.form=17;
        break;
    case 7://Liquid Cp PPDS15
        corr.form=19;
        break;
    case 8://Liquid T from H DIPPR100
        corr.form=140;
        break;
    case 9://Liq. density DIPPR106
        corr.form=47;
        break;
    case 10://Liq. density DIPPR116
        corr.form=46;
        break;
    case 11:{//Liq. Ln of reduced bulk modulus DIPPR100
        int fromRow,toRow;
        corr.form=150;
        QSqlQuery query(db);
        fromRow=ui->leSubsToolsFromRow->text().toInt();
        toRow=ui->leSubsToolsToRow->text().toInt();
        subsData->baseProp.LnuA=(ui->twSubsTools->item(fromRow-1,1)->text().toDouble() - ui->twSubsTools->item(toRow-1,1)->text().toDouble())/
                (ui->twSubsTools->item(fromRow-1,0)->text().toDouble() - ui->twSubsTools->item(toRow-1,0)->text().toDouble());
        subsData->baseProp.LnuB=ui->twSubsTools->item(fromRow-1,1)->text().toDouble() - subsData->baseProp.LnuA*ui->twSubsTools->item(fromRow-1,0)->text().toDouble();
        query.prepare("UPDATE Products SET LnuA=?,LnuB=? WHERE (Id=?)");
            query.addBindValue(subsData->baseProp.LnuA);
            query.addBindValue(subsData->baseProp.LnuB);
            query.addBindValue(subsData->id);
            query.exec();
        break;
    }
    case 12://Not used
        break;
    case 13://Liq. viscosity DIPPR101
        corr.form=30;
        break;
    case 14://Liq. thermal cond. DIPPR100
        corr.form=50;
        break;
    case 15://Liq.surf.tension DIPPR100
        corr.form=60;
        break;
    case 16://Gas sat. density modified Wagner-Pruss
        corr.form=101;
        break;
    case 17://Gas viscosity DIPPR100
        corr.form=111;
        break;
    case 18://Gas thermal cond.DIPPR100
        corr.form=121;
        break;
    case 19://Solid density DIPPR100
        corr.form=71;
        break;
    case 20://Solid Cp DIPPR100
        corr.form=82;
        break;
    }
    AddCorrToDataBase(subsData->id,&corr,&lowLimit,&highLimit,&comment,&db);
}

//Slot for exporting the substance in Modelica FreeFluids.Media format
void FreeFluidsMainWindow::btnSubsToolsExport(){
    QString subsName = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("Substance name (Compatible with Modelica):"));
    QString subsDescription = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("Description for better identification:"));
    QFileDialog *dia = new QFileDialog(this,"Choose directory and file name");
    //dia->setNameFilter("*.mo");
    dia->showNormal();
    QString fileName;
    if (dia->exec())
        fileName = dia->selectedFiles().first();
    std::cout<<fileName.toStdString()<<std::endl;
    QFile *file=new QFile(fileName,this);
    if (file->open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(file);
        out.setRealNumberNotation(QTextStream::ScientificNotation);
        out<<"//To be copied in the FreeFluids.MediaCommon.MediaDataAL/MZ package\n";
        out<<"  constant FreeFluids.MediaCommon.DataRecord "<<subsName.toUtf8()<<"(\n    name = \""<<subsName.toUtf8()<<"\", description = \""<<subsDescription.toUtf8();
        out<<"\", CAS = \""<<subsData->CAS<<"\", family = "<<subsData->baseProp.type<<", MW = "<<subsData->baseProp.MW<<", molarMass = "<<subsData->baseProp.MW*1e-3;
        out<<", Tc = "<<subsData->baseProp.Tc<<", Pc = "<<subsData->baseProp.Pc<<", Vc = "<<subsData->baseProp.Vc<<", Zc = "<<subsData->baseProp.Zc;
        out<<", w = "<<subsData->baseProp.w<<", Tb = "<<subsData->baseProp.Tb;
        if (subsData->baseProp.mu<1.0e2) out<<", mu = "<<subsData->baseProp.mu;
        if (subsData->lIsothComp.y>0) out<<", IsothComp = "<<subsData->lIsothComp.y;
        if (subsData->baseProp.LnuA>0.0) out<<", lnuA = "<<subsData->baseProp.LnuA<<", lnuB = "<<subsData->baseProp.LnuB;

        if (subsData->cp0Corr.form>0){
            out<<",\n    Cp0Corr = "<<subsData->cp0Corr.form<<", Cp0Coef = {"<<subsData->cp0Corr.coef[0]<<", "<<subsData->cp0Corr.coef[1]<<", ";
            out<<subsData->cp0Corr.coef[2]<<", "<<subsData->cp0Corr.coef[3]<<", "<<subsData->cp0Corr.coef[4]<<", "<<subsData->cp0Corr.coef[5]<<", ";
            out<<subsData->cp0Corr.coef[6]<<", "<<subsData->cp0Corr.coef[7]<<", "<<subsData->cp0Corr.coef[8]<<", "<<subsData->cp0Corr.coef[9]<<", ";
            out<<subsData->cp0Corr.coef[10]<<", "<<subsData->cp0Corr.coef[11]<<", "<<subsData->cp0Corr.coef[12]<<"}, ";
            out<<"Cp0LimI = "<<subsData->cp0Corr.limI<<", Cp0LimS = "<<subsData->cp0Corr.limS;
        }
        if (subsData->vpCorr.form>0){
        out<<",\n    VpCorr = "<<subsData->vpCorr.form<<", VpCoef = {"<<subsData->vpCorr.coef[0]<<", "<<subsData->vpCorr.coef[1]<<", ";
        out<<subsData->vpCorr.coef[2]<<", "<<subsData->vpCorr.coef[3]<<", "<<subsData->vpCorr.coef[4]<<", "<<subsData->vpCorr.coef[5]<<"}, ";
        out<<"VpLimI = "<<subsData->vpCorr.limI<<", VpLimS = "<<subsData->vpCorr.limS;
        }
        if (subsData->btCorr.form>0){
        out<<",\n    BtCorr = "<<subsData->btCorr.form<<", BtCoef = {"<<subsData->btCorr.coef[0]<<", "<<subsData->btCorr.coef[1]<<", ";
        out<<subsData->btCorr.coef[2]<<", "<<subsData->btCorr.coef[3]<<", "<<subsData->btCorr.coef[4]<<", "<<subsData->btCorr.coef[5]<<"}, ";
        out<<"BtLimI = "<<subsData->btCorr.limI<<", BtLimS = "<<subsData->btCorr.limS;
        }
        if (subsData->hVsatCorr.form>0){
        out<<",\n    HvCorr = "<<subsData->hVsatCorr.form<<", HvCoef = {"<<subsData->hVsatCorr.coef[0]<<", "<<subsData->hVsatCorr.coef[1]<<", ";
        out<<subsData->hVsatCorr.coef[2]<<", "<<subsData->hVsatCorr.coef[3]<<", "<<subsData->hVsatCorr.coef[4]<<", "<<subsData->hVsatCorr.coef[5]<<"}, ";
        out<<"HvLimI = "<<subsData->hVsatCorr.limI<<", HvLimS = "<<subsData->hVsatCorr.limS;
        }
        if (subsData->lDensCorr.form>0){
        out<<",\n    lDensCorr = "<<subsData->lDensCorr.form<<", lDensCoef = {"<<subsData->lDensCorr.coef[0]<<", "<<subsData->lDensCorr.coef[1]<<", ";
        out<<subsData->lDensCorr.coef[2]<<", "<<subsData->lDensCorr.coef[3]<<", "<<subsData->lDensCorr.coef[4]<<", "<<subsData->lDensCorr.coef[5]<<"}, ";
        out<<"lDensLimI = "<<subsData->lDensCorr.limI<<", lDensLimS = "<<subsData->lDensCorr.limS;
        }
        if (subsData->lCpCorr.form>0){
        out<<",\n    lCpCorr = "<<subsData->lCpCorr.form<<", lCpCoef = {"<<subsData->lCpCorr.coef[0]<<", "<<subsData->lCpCorr.coef[1]<<", ";
        out<<subsData->lCpCorr.coef[2]<<", "<<subsData->lCpCorr.coef[3]<<", "<<subsData->lCpCorr.coef[4]<<", "<<subsData->lCpCorr.coef[5]<<"}, ";
        out<<"lCpLimI = "<<subsData->lCpCorr.limI<<", lCpLimS = "<<subsData->lCpCorr.limS;
        }
        if (subsData->lTfromHCorr.form>0){
        out<<",\n    lTfromHsatCorr = "<<subsData->lTfromHCorr.form<<", lTfromHsatCoef = {"<<subsData->lTfromHCorr.coef[0]<<", "<<subsData->lTfromHCorr.coef[1]<<", ";
        out<<subsData->lTfromHCorr.coef[2]<<", "<<subsData->lTfromHCorr.coef[3]<<", "<<subsData->lTfromHCorr.coef[4]<<", "<<subsData->lTfromHCorr.coef[5]<<"}, ";
        out<<"lTfromHsatLimI = "<<subsData->lTfromHCorr.limI<<", lTfromHsatLimS = "<<subsData->lTfromHCorr.limS;
        }
        if (subsData->lViscCorr.form>0){
        out<<",\n    lViscCorr = "<<subsData->lViscCorr.form<<", lViscCoef = {"<<subsData->lViscCorr.coef[0]<<", "<<subsData->lViscCorr.coef[1]<<", ";
        out<<subsData->lViscCorr.coef[2]<<", "<<subsData->lViscCorr.coef[3]<<", "<<subsData->lViscCorr.coef[4]<<", "<<subsData->lViscCorr.coef[5]<<"}, ";
        out<<"lViscLimI = "<<subsData->lViscCorr.limI<<", lViscLimS = "<<subsData->lViscCorr.limS;
        }
        if (subsData->lThCCorr.form>0){
        out<<",\n    lThCondCorr = "<<subsData->lThCCorr.form<<", lThCondCoef = {"<<subsData->lThCCorr.coef[0]<<", "<<subsData->lThCCorr.coef[1]<<", ";
        out<<subsData->lThCCorr.coef[2]<<", "<<subsData->lThCCorr.coef[3]<<", "<<subsData->lThCCorr.coef[4]<<", "<<subsData->lThCCorr.coef[5]<<"}, ";
        out<<"lThCondLimI = "<<subsData->lThCCorr.limI<<", lThCondLimS = "<<subsData->lThCCorr.limS;
        }
        if (subsData->lSurfTCorr.form>0){
        out<<",\n    lSurfTensCorr = "<<subsData->lSurfTCorr.form<<", lSurfTensCoef = {"<<subsData->lSurfTCorr.coef[0]<<", "<<subsData->lSurfTCorr.coef[1]<<", ";
        out<<subsData->lSurfTCorr.coef[2]<<", "<<subsData->lSurfTCorr.coef[3]<<", "<<subsData->lSurfTCorr.coef[4]<<", "<<subsData->lSurfTCorr.coef[5]<<"}, ";
        out<<"lSurfTensLimI = "<<subsData->lSurfTCorr.limI<<", lSurfTensLimS = "<<subsData->lSurfTCorr.limS;
        }
        if (subsData->lBulkModRCorr.form>0){
        out<<",\n    lBulkModRCorr = "<<subsData->lBulkModRCorr.form<<", lBulkModRCoef = {"<<subsData->lBulkModRCorr.coef[0]<<", "<<subsData->lBulkModRCorr.coef[1]<<", ";
        out<<subsData->lBulkModRCorr.coef[2]<<", "<<subsData->lBulkModRCorr.coef[3]<<", "<<subsData->lBulkModRCorr.coef[4]<<", "<<subsData->lBulkModRCorr.coef[5]<<"}, ";
        out<<"lBulkModRLimI = "<<subsData->lBulkModRCorr.limI<<", lBulkModRLimS = "<<subsData->lBulkModRCorr.limS;
        }
        if (subsData->gDensCorr.form>0){
        out<<",\n    gSatDensCorr = "<<subsData->gDensCorr.form<<", gSatDensCoef = {"<<subsData->gDensCorr.coef[0]<<", "<<subsData->gDensCorr.coef[1]<<", ";
        out<<subsData->gDensCorr.coef[2]<<", "<<subsData->gDensCorr.coef[3]<<", "<<subsData->gDensCorr.coef[4]<<", "<<subsData->gDensCorr.coef[5]<<"}, ";
        out<<"gSatDensLimI = "<<subsData->gDensCorr.limI<<", gSatDensLimS = "<<subsData->gDensCorr.limS;
        }
        if (subsData->gViscCorr.form>0){
        out<<",\n    gViscCorr = "<<subsData->gViscCorr.form<<", gViscCoef = {"<<subsData->gViscCorr.coef[0]<<", "<<subsData->gViscCorr.coef[1]<<", ";
        out<<subsData->gViscCorr.coef[2]<<", "<<subsData->gViscCorr.coef[3]<<", "<<subsData->gViscCorr.coef[4]<<", "<<subsData->gViscCorr.coef[5]<<"}, ";
        out<<"gViscLimI = "<<subsData->gViscCorr.limI<<", gViscLimS = "<<subsData->gViscCorr.limS;
        }
        if (subsData->gThCCorr.form>0){
        out<<",\n    gThCondCorr = "<<subsData->gThCCorr.form<<", gThCondCoef = {"<<subsData->gThCCorr.coef[0]<<", "<<subsData->gThCCorr.coef[1]<<", ";
        out<<subsData->gThCCorr.coef[2]<<", "<<subsData->gThCCorr.coef[3]<<", "<<subsData->gThCCorr.coef[4]<<", "<<subsData->gThCCorr.coef[5]<<"}, ";
        out<<"gThCondLimI = "<<subsData->gThCCorr.limI<<", gThCondLimS = "<<subsData->gThCCorr.limS;
        }
        out<<"); \n\n";

        out<<"//To be copied in the FreeFluids.TMedia.Fluids package. Please change MediaData by MediaDataAL or MediaDataMZ as needed.\n";
        out<<"  package "<<subsName.toUtf8()<<"\n    extends TMedium(final mediumName = \""<<subsName.toUtf8()<<"\", final singleState = false,";
        out<<" fluidConstants = {FreeFluids.MediaCommon.MediaData."<<subsName.toUtf8()<<"}, refState=\"IIR\", reference_T=273.15);\n  end "<<subsName.toUtf8()<<";";

    }
    file->close();
    delete dia;
    delete file;
}

//Slot for storing the reference substance for corresponding states calculation
void FreeFluidsMainWindow::btnSubsToolsKeepRef(){
    QString fileName="kkkkk.kk";
    FILE *outfile;
    outfile = fopen (fileName.toStdString().c_str(),"wb");
    if (outfile == NULL)
    {
        fprintf(stderr, "\nError opend file\n");
        exit (1);
    }
    fwrite (subsData, sizeof(FF_SubstanceData), 1, outfile);
    fclose(outfile);
    outfile = fopen (fileName.toStdString().c_str(),"rb");
    if (outfile == NULL)
    {
        fprintf(stderr, "\nError opend file\n");
        exit (1);
    }
    fread(subsDataRef, sizeof(FF_SubstanceData), 1, outfile);
    fclose(outfile);

    ui->leSubsToolsRefSubs->setText(QString::fromStdString(subsDataRef->name));
}

//Slot for storing propane as reference in corresponding states

void FreeFluidsMainWindow::btnSubsToolsPropaneRef(){
    FF_Correlation corr;

    if (ui->cbSubsToolsSelRefSubs->currentText().toStdString()=="Propane"){
    subsDataRef->id=760;//Id of propane in the database
    GetBasicData(subsDataRef->id,subsDataRef,&db);
    subsDataRef->model=FF_SAFTtype;
    subsDataRef->saftData.id=222;//Id of the SAFT type equation parameters for propane
    GetEOSData(&subsDataRef->model,subsDataRef,&db);
    subsDataRef->model=FF_SWtype;
    subsDataRef->swData.id=845;//Id of the SW equation parameters for propane
    GetEOSData(&subsDataRef->model,subsDataRef,&db);
    corr.id=16372;//Cp0 correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->cp0Corr=corr;
    corr.id=12843;//Liquid visc. correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lViscCorr=corr;
    corr.id=13450;//Liquid th. conductivity correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lThCCorr=corr;
    corr.id=12173;//Vapor pressure correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->vpCorr=corr;
    corr.id=11262;//Liquid saturated density correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lDensCorr=corr;
    }
    else if (ui->cbSubsToolsSelRefSubs->currentText().toStdString()=="R134A"){
    subsDataRef->id=8268;//Id of R134A in the database
    GetBasicData(subsDataRef->id,subsDataRef,&db);
    subsDataRef->model=FF_SAFTtype;
    subsDataRef->saftData.id=998;//Id of the SAFT type equation parameters for R134A
    GetEOSData(&subsDataRef->model,subsDataRef,&db);
    subsDataRef->model=FF_SWtype;
    subsDataRef->swData.id=871;//Id of the SW equation parameters for R134A
    GetEOSData(&subsDataRef->model,subsDataRef,&db);
    corr.id=10772;//Cp0 correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->cp0Corr=corr;
    corr.id=16494;//Liquid visc. correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lViscCorr=corr;
    corr.id=16501;//Liquid th. conductivity correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lThCCorr=corr;
    corr.id=16498;//Vapor pressure correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->vpCorr=corr;
    corr.id=16499;//Liquid saturated density correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lDensCorr=corr;
    }
    else if (ui->cbSubsToolsSelRefSubs->currentText().toStdString()=="Ethylene glycol"){
    subsDataRef->id=61;//Id of Ethylene glycol in the database
    GetBasicData(subsDataRef->id,subsDataRef,&db);
    subsDataRef->model=FF_SAFTtype;
    subsDataRef->saftData.id=952;//Id of the SAFT type equation parameters for EG
    GetEOSData(&subsDataRef->model,subsDataRef,&db);
    corr.id=9011;//Cp0 correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->cp0Corr=corr;
    corr.id=13088;//Liquid visc. correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lViscCorr=corr;
    corr.id=13423;//Liquid th. conductivity correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lThCCorr=corr;
    corr.id=12129;//Vapor pressure correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->vpCorr=corr;
    corr.id=258;//Liquid saturated density correlation
    GetCorrDataById(&corr,&db);
    subsDataRef->lDensCorr=corr;
    }
    ui->leSubsToolsRefSubs->setText(QString::fromStdString(subsDataRef->name));
}

//Slot for performing the corresponding states calculation
void FreeFluidsMainWindow::btnSubsToolsDoCScalc(){
    /*
    double Ttest=260.0;
    double rhoTest=12390.0;
    double eta[3];
    FF_ViscosityTDens(1,&Ttest,&rhoTest,eta);
    printf("etaI:%f etaF:%f etaB:%f etaR:%f eta:%f\n",eta[0],eta[1],eta[2],eta[1]+eta[2],eta[0]+eta[1]+eta[2]);
    return;*/

    double T;
    double result[2];
    int i;
    i=0;
    ui->twSubsTools->horizontalHeaderItem(0)->setText("T(K)");
    ui->twSubsTools->horizontalHeaderItem(1)->setText("liq.visc.(Pa·s)");
    ui->twSubsTools->horizontalHeaderItem(2)->setText("liq.th.cond.(W/m·K)");
    while(ui->twSubsTools->item(i,0)->text()>""){
        T=T=ui->twSubsTools->item(i,0)->text().toDouble();//Selected temperature
        FF_CorrespondingStatesSat(subsData,subsDataRef,&T,result);
        ui->twSubsTools->item(i,1)->setText(QString::number(result[0]));
        ui->twSubsTools->item(i,2)->setText(QString::number(result[1]));
        i++;
    }
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
    eosSel[6]=ui->cbMixCalcEosSelec7->currentIndex();
    eosSel[7]=ui->cbMixCalcEosSelec8->currentIndex();
    eosSel[8]=ui->cbMixCalcEosSelec9->currentIndex();
    eosSel[9]=ui->cbMixCalcEosSelec10->currentIndex();
    eosSel[10]=ui->cbMixCalcEosSelec11->currentIndex();
    eosSel[11]=ui->cbMixCalcEosSelec12->currentIndex();
    eosSel[12]=ui->cbMixCalcEosSelec13->currentIndex();
    eosSel[13]=ui->cbMixCalcEosSelec14->currentIndex();
    eosSel[14]=ui->cbMixCalcEosSelec15->currentIndex();

    cp0Sel[0]=ui->cbMixCalcCp0Selec1->currentIndex();
    cp0Sel[1]=ui->cbMixCalcCp0Selec2->currentIndex();
    cp0Sel[2]=ui->cbMixCalcCp0Selec3->currentIndex();
    cp0Sel[3]=ui->cbMixCalcCp0Selec4->currentIndex();
    cp0Sel[4]=ui->cbMixCalcCp0Selec5->currentIndex();
    cp0Sel[5]=ui->cbMixCalcCp0Selec6->currentIndex();
    cp0Sel[6]=ui->cbMixCalcCp0Selec7->currentIndex();
    cp0Sel[7]=ui->cbMixCalcCp0Selec8->currentIndex();
    cp0Sel[8]=ui->cbMixCalcCp0Selec9->currentIndex();
    cp0Sel[9]=ui->cbMixCalcCp0Selec10->currentIndex();
    cp0Sel[10]=ui->cbMixCalcCp0Selec11->currentIndex();
    cp0Sel[11]=ui->cbMixCalcCp0Selec12->currentIndex();
    cp0Sel[12]=ui->cbMixCalcCp0Selec13->currentIndex();
    cp0Sel[13]=ui->cbMixCalcCp0Selec14->currentIndex();
    cp0Sel[14]=ui->cbMixCalcCp0Selec15->currentIndex();
}

//Write in the results table the thermodynamic records
void FreeFluidsMainWindow::writeMixResultsTable(int nPhases,FF_MixData*mix,FF_ThermoProperties *th0A,FF_PhaseThermoProp *thA,FF_ThermoProperties *th0B,
                                                FF_PhaseThermoProp *thB, FF_ThermoProperties *th0C,FF_PhaseThermoProp *thC){
    int i,j;
    double cm;//Massic fraction
    //we clear the content of the table
    //for (i=0;i<ui->twMixCalc->rowCount();i++) for (j=0;j<ui->twMixCalc->columnCount();j++) ui->twMixCalc->item(i,j)->setText("");
    ui->twMixCalc->item(0,1)->setText(QString::number(thA->MW));
    ui->twMixCalc->item(1,1)->setText(QString::number(thA->P*0.00001));
    ui->twMixCalc->item(2,1)->setText(QString::number(thA->T));
    ui->twMixCalc->item(3,1)->setText(QString::number(thA->fraction));
    if(mix->thModelActEos!=2){
        ui->twMixCalc->item(7,1)->setText(QString::number(thA->V*1e6));//Molar volume cm3/mol
        ui->twMixCalc->item(8,1)->setText(QString::number(thA->MW/thA->V*0.001));//rho kgr/m3
    }
    ui->twMixCalc->item(9,1)->setText(QString::number(th0A->H/th0A->MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,1)->setText(QString::number(th0A->S/th0A->MW));
    ui->twMixCalc->item(11,1)->setText(QString::number(th0A->Cp/th0A->MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,1)->setText(QString::number(th0A->Cv/th0A->MW));//Cv0 KJ/kgr·K
    if(mix->thModelActEos!=2){
        ui->twMixCalc->item(13,1)->setText(QString::number(thA->H/thA->MW));
        ui->twMixCalc->item(14,1)->setText(QString::number(thA->U/thA->MW));
        ui->twMixCalc->item(15,1)->setText(QString::number(thA->S/thA->MW));
        ui->twMixCalc->item(16,1)->setText(QString::number(thA->Cp/thA->MW));
        ui->twMixCalc->item(17,1)->setText(QString::number(thA->Cv/thA->MW));
        ui->twMixCalc->item(18,1)->setText(QString::number(thA->SS));
        ui->twMixCalc->item(19,1)->setText(QString::number(thA->JT*1e5));
        ui->twMixCalc->item(20,1)->setText(QString::number(thA->IT*1e2));
        ui->twMixCalc->item(21,1)->setText(QString::number(thA->dP_dT*1e-5));
        ui->twMixCalc->item(22,1)->setText(QString::number(thA->dP_dV*1e-5));
        ui->twMixCalc->item(23,1)->setText(QString::number(thA->ArrDer[0]));
        ui->twMixCalc->item(24,1)->setText(QString::number(thA->ArrDer[1]));
        ui->twMixCalc->item(25,1)->setText(QString::number(thA->ArrDer[2]));
        ui->twMixCalc->item(26,1)->setText(QString::number(thA->ArrDer[3]));
        ui->twMixCalc->item(27,1)->setText(QString::number(thA->ArrDer[4]));
        ui->twMixCalc->item(28,1)->setText(QString::number(thA->ArrDer[5]));
    }
    for(i=0;i<mix->numSubs;i++){
        ui->twMixCalc->item(29+i,1)->setText(QString::number(thA->c[i]));//Mole concentrations
        cm=thA->c[i]*mix->baseProp[i].MW/thA->MW;
        ui->twMixCalc->item(44+i,1)->setText(QString::number(cm));
        ui->twMixCalc->item(59+i,1)->setText(QString::number(thA->subsPhi[i]));
    }
    //Second phase
    ui->twMixCalc->item(0,2)->setText(QString::number(thB->MW));
    ui->twMixCalc->item(1,2)->setText(QString::number(thB->P*0.00001));
    ui->twMixCalc->item(2,2)->setText(QString::number(thB->T));
    ui->twMixCalc->item(3,2)->setText(QString::number(thB->fraction));
    if(mix->thModelActEos==1){
        ui->twMixCalc->item(7,2)->setText(QString::number(thB->V*1e6));//Molar volume cm3/mol
        ui->twMixCalc->item(8,2)->setText(QString::number(thB->MW/thB->V*0.001));//rho kgr/m3
    }
    ui->twMixCalc->item(9,2)->setText(QString::number(th0B->H/th0B->MW));//H0 KJ/kgr
    ui->twMixCalc->item(10,2)->setText(QString::number(th0B->S/th0B->MW));
    ui->twMixCalc->item(11,2)->setText(QString::number(th0B->Cp/th0B->MW));//Cp0 KJ/kgr·K
    ui->twMixCalc->item(12,2)->setText(QString::number(th0B->Cv/th0B->MW));//Cv0 KJ/kgr·K
    if(mix->thModelActEos==1){
        ui->twMixCalc->item(13,2)->setText(QString::number(thB->H/thB->MW));
        ui->twMixCalc->item(14,2)->setText(QString::number(thB->U/thB->MW));
        ui->twMixCalc->item(15,2)->setText(QString::number(thB->S/thB->MW));
        ui->twMixCalc->item(16,2)->setText(QString::number(thB->Cp/thB->MW));
        ui->twMixCalc->item(17,2)->setText(QString::number(thB->Cv/thB->MW));
        ui->twMixCalc->item(18,2)->setText(QString::number(thB->SS));
        ui->twMixCalc->item(19,2)->setText(QString::number(thB->JT*1e5));
        ui->twMixCalc->item(20,2)->setText(QString::number(thB->IT*1e2));
        ui->twMixCalc->item(21,2)->setText(QString::number(thB->dP_dT*1e-5));
        ui->twMixCalc->item(22,2)->setText(QString::number(thB->dP_dV*1e-5));
        ui->twMixCalc->item(23,2)->setText(QString::number(thB->ArrDer[0]));
        ui->twMixCalc->item(24,2)->setText(QString::number(thB->ArrDer[1]));
        ui->twMixCalc->item(25,2)->setText(QString::number(thB->ArrDer[2]));
        ui->twMixCalc->item(26,2)->setText(QString::number(thB->ArrDer[3]));
        ui->twMixCalc->item(27,2)->setText(QString::number(thB->ArrDer[4]));
        ui->twMixCalc->item(28,2)->setText(QString::number(thB->ArrDer[5]));
    }
    for(i=0;i<mix->numSubs;i++){
        ui->twMixCalc->item(29+i,2)->setText(QString::number(thB->c[i]));//Mole concentrations
        cm=thB->c[i]*mix->baseProp[i].MW/thB->MW;
        ui->twMixCalc->item(44+i,2)->setText(QString::number(cm));
        ui->twMixCalc->item(59+i,2)->setText(QString::number(thB->subsPhi[i]));
    }
    if(nPhases>2){
        ui->twMixCalc->item(0,3)->setText(QString::number(thC->MW));
        ui->twMixCalc->item(1,3)->setText(QString::number(thC->P*0.00001));
        ui->twMixCalc->item(2,3)->setText(QString::number(thC->T));
        ui->twMixCalc->item(3,3)->setText(QString::number(thC->fraction));
        if(mix->thModelActEos==1){
            ui->twMixCalc->item(7,3)->setText(QString::number(thC->V*1e6));//Molar volume cm3/mol
            ui->twMixCalc->item(8,3)->setText(QString::number(thC->MW/thC->V*0.001));//rho kgr/m3
        }
        ui->twMixCalc->item(9,3)->setText(QString::number(th0C->H/th0C->MW));//H0 KJ/kgr
        ui->twMixCalc->item(10,3)->setText(QString::number(th0C->S/th0C->MW));
        ui->twMixCalc->item(11,3)->setText(QString::number(th0C->Cp/th0C->MW));//Cp0 KJ/kgr·K
        ui->twMixCalc->item(12,3)->setText(QString::number(th0C->Cv/th0C->MW));//Cv0 KJ/kgr·K
        if(mix->thModelActEos==1){
            ui->twMixCalc->item(13,3)->setText(QString::number(thC->H/thC->MW));
            ui->twMixCalc->item(14,3)->setText(QString::number(thC->U/thC->MW));
            ui->twMixCalc->item(15,3)->setText(QString::number(thC->S/thC->MW));
            ui->twMixCalc->item(16,3)->setText(QString::number(thC->Cp/thC->MW));
            ui->twMixCalc->item(17,3)->setText(QString::number(thC->Cv/thC->MW));
            ui->twMixCalc->item(18,3)->setText(QString::number(thC->SS));
            ui->twMixCalc->item(19,3)->setText(QString::number(thC->JT*1e5));
            ui->twMixCalc->item(20,3)->setText(QString::number(thC->IT*1e2));
            ui->twMixCalc->item(21,3)->setText(QString::number(thC->dP_dT*1e-5));
            ui->twMixCalc->item(22,3)->setText(QString::number(thC->dP_dV*1e-5));
            ui->twMixCalc->item(23,3)->setText(QString::number(thC->ArrDer[0]));
            ui->twMixCalc->item(24,3)->setText(QString::number(thC->ArrDer[1]));
            ui->twMixCalc->item(25,3)->setText(QString::number(thC->ArrDer[2]));
            ui->twMixCalc->item(26,3)->setText(QString::number(thC->ArrDer[3]));
            ui->twMixCalc->item(27,3)->setText(QString::number(thC->ArrDer[4]));
            ui->twMixCalc->item(28,3)->setText(QString::number(thC->ArrDer[5]));
        }
        for(i=0;i<mix->numSubs;i++){
            ui->twMixCalc->item(29+i,3)->setText(QString::number(thC->c[i]));//Mole concentrations
            cm=thC->c[i]*mix->baseProp[i].MW/thC->MW;
            ui->twMixCalc->item(44+i,3)->setText(QString::number(cm));//Mass concentrations
            ui->twMixCalc->item(59+i,3)->setText(QString::number(thC->subsPhi[i]));
        }
    }
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
    mix->numSubs=0;
    while ((ui->twMixComposition->item( mix->numSubs,0)->text().toInt()>0)&&( mix->numSubs<(ui->twMixComposition->rowCount()-1)))  mix->numSubs++;//we count the filled rows
}

//Slot for clearing content in the table
void  FreeFluidsMainWindow::twMixCompositionClear(){
    int i,j;//the loop variables
    //we clear the content of the composition table
    for (i=0;i<ui->twMixComposition->rowCount();i++) for (j=0;j<ui->twMixComposition->columnCount();j++) ui->twMixComposition->item(i,j)->setText("");
    mix->numSubs=0;
    ui->leMixCalcMWmix->setText("");
    ui->cbMixCalcEosTypeSelec->setCurrentIndex(0);
}

//Slot for calculation of mass and molar fractions in the composition table
void  FreeFluidsMainWindow::twMixCompositionCalcFract(){
    unsigned j;
    bool mass;
    //mix->numSubs=0;
    //while ((ui->twMixComposition->item( mix->numSubs,3)->text().toDouble()>0)&&( mix->numSubs<(ui->twMixComposition->rowCount()-1)))  mix->numSubs++;//we count the filled rows
    double MW[ mix->numSubs],q[ mix->numSubs],massFrac[ mix->numSubs],moleFrac[ mix->numSubs];
    double MWmix=0;
    for (j=0;j< mix->numSubs;j++){
        MW[j]=ui->twMixComposition->item(j,2)->text().toDouble();
        q[j]=ui->twMixComposition->item(j,3)->text().toDouble();
    }
    if (ui->cbMixCompositionMass->isChecked()==true) mass=true;
    else mass=false;
    FF_FractionsCalculation( mix->numSubs, MW, q, mass, massFrac, moleFrac);//we obtain the fractions
    for (j=0;j< mix->numSubs;j++){//We fill the table with the results and calculate the mix molecular weight
        ui->twMixComposition->item(j,4)->setText(QString::number(massFrac[j]));
        ui->twMixComposition->item(j,5)->setText(QString::number(moleFrac[j]));
        MWmix=MWmix+MW[j]*moleFrac[j];
    }
    ui->leMixCalcMWmix->setText(QString::number(MWmix));//We fill the molecular weight of the mixture
}

//Slot for updating the general thermo model
void  FreeFluidsMainWindow::cbMixCalcLiqModelLoad(int position){
    if(position==0) mix->thModelActEos=0;
    else if(position==1) mix->thModelActEos=1;
    else mix->thModelActEos=2;
}


//Slot for loading the existing EOS and Cp0 correlations for the selected substances
void  FreeFluidsMainWindow::cbMixCalcEosTypeLoad(int position){
    QSqlQuery queryEos,queryCp0;
    QString eosTypeQs;
    unsigned i=0,j;
    switch(position){
    case 0:
        mix->eosType=FF_NoType;
        eosTypeQs="None";
        break;
    case 1:
        mix->eosType=FF_CubicPRtype;
        eosTypeQs="Cubic PR";
        break;
    case 2:
        mix->eosType=FF_CubicSRKtype;
        eosTypeQs="Cubic SRK";
        break;
    case 3:
        mix->eosType=FF_SAFTtype;
        eosTypeQs="SAFT";
        break;
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
        tvMixCalcSelEOS[j]->setColumnWidth(0,96);
        tvMixCalcSelEOS[j]->setColumnWidth(1,356);
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
        mix->mixRule=FF_NoMixRul;
        break;
    case 1:
        mix->mixRule=FF_VdWnoInt;
        break;
    case 2:
        mix->mixRule=FF_VdW;
        break;
    case 3:
        mix->mixRule=FF_PR;
        break;
    case 4:
        mix->mixRule=FF_MKP;
        break;
    case 5:
        mix->mixRule=FF_HV;
        break;
    case 6:
        mix->mixRule=FF_MHV1;
        break;
    case 7:
        mix->mixRule=FF_PSRK;
        break;
    case 8:
        mix->mixRule=FF_LCVM;
        break;
    case 9:
        mix->mixRule=FF_MHV2;
        break;
    case 10:
        mix->mixRule=FF_UMR;
        break;
    case 11:
        mix->mixRule=FF_BL;
        break;
    }
}

//Slot for storing the selected activity model
void FreeFluidsMainWindow::cbMixCalcActModelLoad(int row){
    switch(row){//We charge the selecte mixing rule
    case 0:
        mix->actModel=FF_NoModel;
        break;
    case 1:
        mix->actModel=FF_Wilson;
        break;
    case 2:
        mix->actModel=FF_NRTL;
        break;
    case 3:
        mix->actModel=FF_UNIQUAC;
        break;
    case 4:
        mix->actModel=FF_UNIQUACFV;
        break;
    case 5:
        mix->actModel=FF_UNIFACStd;
        break;
    case 6:
        mix->actModel=FF_UNIFACPSRK;
        break;
    case 7:
        mix->actModel=FF_UNIFACDort;
        break;
    case 8:
        mix->actModel=FF_UNIFACNist;
        break;
    case 9:
        mix->actModel=FF_EntropicFV;
        break;
    case 10:
        mix->actModel=FF_UNIFACZM;
        break;
    case 11:
        mix->actModel=FF_Hildebrand;
        break;
    case 12:
        mix->actModel=FF_Hansen;
        break;
    case 13:
        mix->actModel=FF_Chi;
        break;
    }

}


//Slot for displaying the intParam array for eos, for the selecte pair, and charge the combobox of available interaction parameters
void FreeFluidsMainWindow::twMixIntParamEosDisplay(int row,int column){
    //We display in the table the actual interaction parameters
    for(int i=0;i<6;i++){
        ui->twMixIntParamEos->item(0,i)->setText(QString::number(mix->intParam[row][column][i]));
        ui->twMixIntParamEos->item(1,i)->setText(QString::number(mix->intParam[column][row][i]));
    }
    //We update the combobox with the available interactions parameters available to choose for the pair

    QString eosTypeQs,mixRuleQs,actModelQs;
    QSqlQuery queryIntParam;
    switch(mix->mixRule){
    case FF_NoMixRul:
        mixRuleQs="None";
        break;
    case FF_VdW:
        mixRuleQs="VdW";
        break;
    case FF_PR:
        mixRuleQs="PR";
        break;
    case FF_MKP:
        mixRuleQs="MKP";
        break;
    case FF_BL:
        mixRuleQs="BL";
        break;
    }
    switch(mix->eosType){
    case FF_CubicPRtype:
        eosTypeQs="Cubic PR";
        break;
    case FF_CubicSRKtype:
        eosTypeQs="Cubic SRK";
        break;
    case FF_SAFTtype:
        eosTypeQs="SAFT";
        break;
    }
    switch(mix->actModel){
    case FF_Wilson:
        actModelQs="Wilson";
        break;
    case FF_NRTL:
        actModelQs="NRTL";
        break;
    case FF_UNIQUAC:
        actModelQs="UNIQUAC";
        break;
    }
    //printf("thModel:%i mixRule:%i actModel:%i\n",mix->thModelActEos,mix->mixRule,mix->actModel);
    if((mix->thModelActEos==1)&&((mix->mixRule==FF_NoMixRul)||(mix->mixRule==FF_VdW)||(mix->mixRule==FF_PR)||(mix->mixRule==FF_MKP)||(mix->mixRule==FF_BL))){
            queryIntParam.prepare("SELECT EosInteraction.* FROM EosInteraction  WHERE ((IdProduct1=?) AND (IdProduct2=?) AND (EosType=?) AND (EosInteraction.Rule=?))");
            queryIntParam.bindValue(0,ui->twMixComposition->item(row,0)->text().toInt());
            queryIntParam.bindValue(1,ui->twMixComposition->item(column,0)->text().toInt());
            queryIntParam.bindValue(2,eosTypeQs);
            queryIntParam.bindValue(3,mixRuleQs);
            queryIntParam.exec();
            mixIntParamSelModel->setQuery(queryIntParam);
            tvMixIntParamSel->setColumnWidth(0,1);
            tvMixIntParamSel->setColumnWidth(20,250);
            //tvMixIntParamSel->setColumnHidden(0,true);
            for(int i=6;i<20;i++) tvMixIntParamSel->setColumnHidden(i,true);
            ui->cbMixIntParamSel->setModelColumn(20);
    }
    else if((mix->actModel==FF_Wilson)||(mix->actModel==FF_NRTL)||(mix->actModel==FF_UNIQUAC)){
        queryIntParam.prepare("SELECT ActInteraction.* FROM ActInteraction  WHERE ((IdProduct1=?) AND (IdProduct2=?) AND (Model=?))");
        //printf("hi trying to charge\n");
        queryIntParam.bindValue(0,ui->twMixComposition->item(row,0)->text().toInt());
        queryIntParam.bindValue(1,ui->twMixComposition->item(column,0)->text().toInt());
        queryIntParam.bindValue(2,actModelQs);
        queryIntParam.exec();
        mixIntParamSelModel->setQuery(queryIntParam);
        tvMixIntParamSel->setColumnWidth(0,1);
        tvMixIntParamSel->setColumnWidth(19,250);
        //tvMixIntParamSel->setColumnHidden(0,true);
        for(int i=5;i<19;i++) tvMixIntParamSel->setColumnHidden(i,true);
        ui->cbMixIntParamSel->setModelColumn(19);
    }

}

//Slot for filling the intParam array, and tablewidget, for eos, from the record already charged from the database
void FreeFluidsMainWindow::twMixIntParamEosFill(int row){
    int i,j;
    i=ui->twMixPairSel->currentRow();
    j=ui->twMixPairSel->currentColumn();
    //update array
    mix->intParam[i][j][0]=mixIntParamSelModel->record(row).value("Param1").toFloat();
    mix->intParam[i][j][1]=mixIntParamSelModel->record(row).value("Param2").toFloat();
    mix->intParam[i][j][2]=mixIntParamSelModel->record(row).value("Param3").toFloat();
    mix->intParam[i][j][3]=mixIntParamSelModel->record(row).value("Param4").toFloat();
    mix->intParam[i][j][4]=mixIntParamSelModel->record(row).value("Param5").toFloat();
    mix->intParam[i][j][5]=mixIntParamSelModel->record(row).value("Param6").toFloat();

    mix->intParam[j][i][0]=mixIntParamSelModel->record(row).value("Param1i").toFloat();
    mix->intParam[j][i][1]=mixIntParamSelModel->record(row).value("Param2i").toFloat();
    mix->intParam[j][i][2]=mixIntParamSelModel->record(row).value("Param3i").toFloat();
    mix->intParam[j][i][3]=mixIntParamSelModel->record(row).value("Param4i").toFloat();
    mix->intParam[j][i][4]=mixIntParamSelModel->record(row).value("Param5i").toFloat();
    mix->intParam[j][i][5]=mixIntParamSelModel->record(row).value("Param6i").toFloat();
    //Update formula to use
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol1"))mix->intForm=FF_Pol1;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol1C"))mix->intForm=FF_Pol1C;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol1J"))mix->intForm=FF_Pol1J;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol1K"))mix->intForm=FF_Pol1K;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol2"))mix->intForm=FF_Pol2;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol2C"))mix->intForm=FF_Pol2C;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol2J"))mix->intForm=FF_Pol2J;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol2K"))mix->intForm=FF_Pol2K;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol3"))mix->intForm=FF_Pol3;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol3C"))mix->intForm=FF_Pol3C;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol3J"))mix->intForm=FF_Pol3J;
    if((mixIntParamSelModel->record(row).value("Equation").toString().toStdString()=="Pol3K"))mix->intForm=FF_Pol3K;

    //update tablewidget to see the parameters
    for(int k=0;k<6;k++){
        ui->twMixIntParamEos->item(0,k)->setText(QString::number(mix->intParam[i][j][k]));
        ui->twMixIntParamEos->item(1,k)->setText(QString::number(mix->intParam[j][i][k]));
    }
    ui->twMixIntParamEos->item(0,6)->setText(mixIntParamSelModel->record(row).value("Equation").toString());
    ui->twMixIntParamEos->item(0,7)->setText(mixIntParamSelModel->record(row).value("Description").toString());
    ui->twMixIntParamEos->setColumnWidth(7,257);
}


//Slot for updating the interaction parameter array for eos, from the display
void FreeFluidsMainWindow::twMixIntParamEosUpdate(int row,int column){
    int i,j;
    if (column<6){
        i=ui->twMixPairSel->currentRow();
        j=ui->twMixPairSel->currentColumn();
        if (row==0) mix->intParam[i][j][column]=ui->twMixIntParamEos->item(row,column)->text().toFloat();
        else if (row==1) mix->intParam[j][i][column]=ui->twMixIntParamEos->item(row,column)->text().toFloat();
        //printf("%f\n",mix->intParam[i][j][column]);
    }
}


//Slot for clearing all interaction parameters
void FreeFluidsMainWindow::twMixIntParamClear(){
        int i,j,k;
        mix->intForm=0;
        for(i=0;i<15;i++)for(j=0;j<15;j++)for(k=0;k<6;k++){
            mix->intParam[i][j][k]=0;
        }
        //We clear also the interaction parameters table
        for(int i=0;i<8;i++){
            ui->twMixIntParamEos->item(0,i)->setText("");
            ui->twMixIntParamEos->item(1,i)->setText("");
        }
}

//Slot for substances and mixture creation
void FreeFluidsMainWindow::btnMixCalcCreateSys(){
    substance= new FF_SubstanceData[15];

    QString type;
    int i;//the loop variable

    getMixEosCpSel();//gives Invalid parameter passed message

    //Read the selection made for the substances and get their data
    for (i=0;i< mix->numSubs;i++){
        substance[i].id=ui->twMixComposition->item(i,0)->text().toInt();
        strcpy(substance[i].name,ui->twMixComposition->item(i,1)->text().toStdString().c_str());
        //printf("subst[%i]\n",substance[i].id);
        GetBasicData(substance[i].id,&substance[i],&db);
        //printf("MW:%f\n",substance[i].baseProp.MW);

        if((mix->eosType==FF_CubicType)||(mix->eosType==FF_CubicPRtype)||(mix->eosType==FF_CubicSRKtype))
            substance[i].cubicData.id=mixCalcEOSModel[i]->record(eosSel[i]).value("Id").toInt();
        else if(mix->eosType==FF_SAFTtype)substance[i].saftData.id=mixCalcEOSModel[i]->record(eosSel[i]).value("Id").toInt();
        else if(mix->eosType==FF_SWtype)substance[i].swData.id=mixCalcEOSModel[i]->record(eosSel[i]).value("Id").toInt();
        GetEOSData(&mix->eosType,&substance[i],&db);
        //printf("Tc:%f\n",substance[i].saftData.Tc);

        substance[i].cp0Corr.id=mixCalcCp0Model[i]->record(cp0Sel[i]).value("Id").toInt();
        GetCorrDataById(&substance[i].cp0Corr,&db);
        //printf("A:%f\n",substance[i].cp0Corr.coef[0]);




        type="Vp";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].vpCorr.form,substance[i].vpCorr.coef);
        //printf("A:%f\n",substance[i].vpCorr.coef[0]);

        type="Ldens";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].lDensCorr.form,substance[i].lDensCorr.coef);
        //printf("A:%f\n",substance[i].lDensCorr.coef[0]);

        type="LCp";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].lDensCorr.form,substance[i].lDensCorr.coef);
        //printf("A:%f\n",substance[i].lCpCorr.coef[0]);

        type="Lvisc";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].lViscCorr.form,substance[i].lViscCorr.coef);
        //printf("A:%f\n",substance[i].lViscCorr.coef[0]);

        type="LthC";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].lThCCorr.form,substance[i].lThCCorr.coef);
        //printf("A:%f\n",substance[i].lThCCorr.coef[0]);

        type="LsurfT";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].lSurfTCorr.form,substance[i].lSurfTCorr.coef);
        //printf("A:%f\n",substance[i].lSurfTCorr.coef[0]);

        type="Gvisc";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].gViscCorr.form,substance[i].gViscCorr.coef);
        //printf("A:%f\n",substance[i].gViscCorr.coef[0]);

        type="GthC";
        GetCorrDataByType(&substance[i].id,&type,&db,&substance[i].gThCCorr.form,substance[i].gThCCorr.coef);
        //printf("A:%f\n",substance[i].gThCCorr.coef[0]);

    }
    subsPoint= new FF_SubstanceData*[15];
    for(i=0;i<15;i++)subsPoint[i]=&substance[i];

    FF_MixFillDataWithSubsData(&mix->numSubs,subsPoint,mix);

    if (ui->cbMixCalcRefPhiSelec->currentIndex()==0) mix->refVpEos=0;
    else mix->refVpEos=1;

    //printf("de mix R:%f\n",mix->unifDortData.subsR[0]);
    delete[] subsPoint;
    delete[] substance;
}

//Slot for mixture exportation
void FreeFluidsMainWindow::btnMixCalcExportMix(){
    int i;
    double c[15];
    for(i=0;i<15;i++){
        c[i]=0;
    }
    for (i=0;i< mix->numSubs;i++){
        c[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }
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
    fwrite (mix, sizeof(FF_MixData), 1, outfile);
    fwrite (c, sizeof(double), 15, outfile);
    fclose(outfile);
    delete dia;
}

//Slot for mixture importation
void FreeFluidsMainWindow::btnMixCalcImportMix(){
    int i,j;
    double c[15];
    QFileDialog *dia = new QFileDialog(this,"Choose directory and file name");
    dia->showNormal();
    QString fileName;
    if (dia->exec())
        fileName = dia->selectedFiles().first();

    FILE *infile;
    // open file for reading
    infile = fopen (fileName.toStdString().c_str(),"rb");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opend file\n");
        delete dia;
        exit (1);
    }
    fread(mix, sizeof(FF_MixData), 1, infile);
    fread(c, sizeof(double), 15, infile);
    fclose(infile);
    delete dia;

    //Now it is necessary to show the content of mix in the interface
    //clears the content of the composition table
    for (i=0;i<ui->twMixComposition->rowCount();i++) for (j=0;j<ui->twMixComposition->columnCount();j++) ui->twMixComposition->item(i,j)->setText("");
    ui->cbMixCalcLiqModelSelec->setCurrentIndex(mix->thModelActEos);
    switch(mix->eosType){
    case FF_NoType:
        ui->cbMixCalcEosTypeSelec->setCurrentIndex(0);
        break;
    case FF_CubicPRtype:
        ui->cbMixCalcEosTypeSelec->setCurrentIndex(1);
        break;
    case FF_CubicSRKtype:
        ui->cbMixCalcEosTypeSelec->setCurrentIndex(2);
        break;
    case FF_SAFTtype:
        ui->cbMixCalcEosTypeSelec->setCurrentIndex(3);
        break;
    }
    switch(mix->mixRule){
    case FF_NoMixRul:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(0);
        break;
    case FF_VdWnoInt:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(1);
        break;
    case FF_VdW:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(2);
        break;
    case FF_PR:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(3);
        break;
    case FF_MKP:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(4);
        break;
    case FF_HV:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(5);
        break;
    case FF_MHV1:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(6);
        break;
    case FF_PSRK:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(7);
        break;
    case FF_LCVM:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(8);
        break;
    case FF_MHV2:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(9);
        break;
    case FF_UMR:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(10);
        break;
    case FF_BL:
        ui->cbMixCalcMixRuleSelec->setCurrentIndex(11);
        break;
    }
    switch(mix->actModel){
    case FF_NoModel:
        ui->cbMixCalcActModelSelec->setCurrentIndex(0);
        break;
    case FF_Wilson:
        ui->cbMixCalcActModelSelec->setCurrentIndex(1);
        break;
    case FF_NRTL:
        ui->cbMixCalcActModelSelec->setCurrentIndex(2);
        break;
    case FF_UNIQUAC:
        ui->cbMixCalcActModelSelec->setCurrentIndex(3);
        break;
    case FF_UNIQUACFV:
        ui->cbMixCalcActModelSelec->setCurrentIndex(4);
        break;
    case FF_UNIFACStd:
        ui->cbMixCalcActModelSelec->setCurrentIndex(5);
        break;
    case FF_UNIFACPSRK:
        ui->cbMixCalcActModelSelec->setCurrentIndex(6);
        break;
    case FF_UNIFACDort:
        ui->cbMixCalcActModelSelec->setCurrentIndex(7);
        break;
    case FF_UNIFACNist:
        ui->cbMixCalcActModelSelec->setCurrentIndex(8);
        break;
    case FF_EntropicFV:
        ui->cbMixCalcActModelSelec->setCurrentIndex(9);
        break;
    case FF_Hildebrand:
        ui->cbMixCalcActModelSelec->setCurrentIndex(10);
        break;
    case FF_Hansen:
        ui->cbMixCalcActModelSelec->setCurrentIndex(11);
        break;
    case FF_Chi:
        ui->cbMixCalcActModelSelec->setCurrentIndex(12);
        break;
    }
    for(i=0;i<mix->numSubs;i++){
        ui->twMixComposition->item(i,0)->setText(QString::number(mix->id[i]));//add Id
        ui->twMixComposition->item(i,1)->setText(QString(mix->subsName[i]));//add name
        ui->twMixComposition->item(i,2)->setText(QString::number(mix->baseProp[i].MW));//add MW
        ui->twMixComposition->item(i,3)->setText(QString::number(c[i]));
        ui->twMixComposition->item(i,5)->setText(QString::number(c[i]));
    }
    if(mix->refVpEos==0)ui->cbMixCalcRefPhiSelec->setCurrentIndex(0);
    else ui->cbMixCalcRefPhiSelec->setCurrentIndex(1);

    cbMixCalcEosTypeLoad( ui->cbMixCalcEosTypeSelec->currentIndex());//shows the available eos for each substance
    /*
    */
}

//Slot for mixture bubble P calculation, and display in table
void FreeFluidsMainWindow::twMixCalcBubbleP(){
    //printf("Act/Eos: %i Mixrule: %i ActModel: %i form: %i 0: %f 1: %f\n",mix->thModelActEos,mix->mixRule,mix->actModel,mix->intForm,mix->intParam[0][1][0],mix->intParam[0][1][1]);
    int i;//the loop variable
    int nPhases=2;
    FF_ThermoProperties th0l,th0g,th0C;//for ideal gas properties. C wil not be used
    FF_PhaseThermoProp thl,thg,thC;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    double Z,Zg;
    double bubbleP;
    double bubblePguess=ui->leMixCalcPresGuess->text().toDouble()*1e5;

    for(i=0;i<15;i++){
        thl.c[i]=thl.subsPhi[i]=thg.c[i]=thg.subsPhi[i]=0;
    }
    //we clear the content of the results table
    for (i=0;i<ui->twMixCalc->rowCount();i++){
        ui->twMixCalc->item(i,1)->setText("");
        ui->twMixCalc->item(i,2)->setText("");
    }

    //Now we need to read the selections made for the molar fractions
    for (i=0;i< mix->numSubs;i++){
        thl.c[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }

    //And begin the calculations
    th0l.MW=thl.MW=MW;
    th0l.T=thl.T=th0g.T=thg.T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    FF_BubbleP(mix,&thl.T,thl.c,&bubblePguess,&bubbleP,thg.c,thl.subsPhi,thg.subsPhi);
    thg.MW=0;
    for(i=0;i<mix->numSubs;i++)thg.MW=thg.MW+thg.c[i]*mix->baseProp[i].MW;
    th0g.MW=thg.MW;
    th0l.P=thl.P=th0g.P=thg.P=bubbleP;
    thl.fraction=1;
    thg.fraction=0;


    option='l';//determine only the liquid volume
    FF_MixVfromTPeos(mix,&thl.T,&thl.P,thl.c,&option,answerL,answerG,&state);
    //printf("state:%c P:%f Vl:%f Vg:%f\n",state,thl.P,answerL[0],answerG[0]);
    th0l.V=answerL[0];
    thl.V=answerL[0];
    Z=answerL[2];
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);
    phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thl.c,&refT,&refP,&th0l);
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);
    FF_MixThermoEOS(mix,&refT,&refP,&thl);
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);


    option='g';//determine only the gas volume
    FF_MixVfromTPeos(mix,&thg.T,&thg.P,thg.c,&option,answerL,answerG,&state);
    th0g.V=thg.V=answerG[0];
    Zg=answerG[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thg.c,&refT,&refP,&th0g);
    FF_MixThermoEOS(mix,&refT,&refP,&thg);
    phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thl.T,thl.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);
    /*
    */
    writeMixResultsTable(nPhases,mix,&th0g,&thg,&th0l,&thl,&th0C,&thC);
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));

    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
}


//Slot for mixture dew P calculation, and display in table
void FreeFluidsMainWindow::twMixCalcDewP(){
    int i;//the loop variable
    int nPhases=2;
    FF_ThermoProperties th0l,th0g,th0C;
    FF_PhaseThermoProp thl,thg,thC;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    double Z,Zg;
    double dewP;
    double dewPguess=ui->leMixCalcPresGuess->text().toDouble()*1e5;

    for (i=0;i<ui->twMixCalc->rowCount();i++){//we clear the content of the results table
        ui->twMixCalc->item(i,1)->setText("");
        ui->twMixCalc->item(i,2)->setText("");
    }

    //Now we need to read the selections made for the molar fraction
    for (i=0;i< mix->numSubs;i++){
        thg.c[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }

    //Now we begin the calculations
    th0g.MW=thg.MW=MW;
    th0l.T=thl.T=th0g.T=thg.T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    FF_DewP(mix,&thg.T,thg.c,&dewPguess,&dewP,thl.c,thl.subsPhi,thg.subsPhi);
    thl.MW=0;
    for(i=0;i<mix->numSubs;i++)thl.MW=thl.MW+thl.c[i]*mix->baseProp[i].MW;
    th0l.MW=thl.MW;
    th0l.P=thl.P=th0g.P=thg.P=dewP;
    thl.fraction=0;
    thg.fraction=1;

    option='l';//determine only the liquid volume
    FF_MixVfromTPeos(mix,&thl.T,&thl.P,thl.c,&option,answerL,answerG,&state);
    th0l.V=thl.V=answerL[0];
    Z=answerL[2];
    phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thl.c,&refT,&refP,&th0l);
    FF_MixThermoEOS(mix,&refT,&refP,&thl);


    option='g';//determine only the gas volume
    FF_MixVfromTPeos(mix,&thg.T,&thg.P,thg.c,&option,answerL,answerG,&state);
    th0g.V=thg.V=answerG[0];
    Zg=answerG[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thg.c,&refT,&refP,&th0g);
    FF_MixThermoEOS(mix,&refT,&refP,&thg);
    phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thl.T,thl.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);

    writeMixResultsTable(nPhases,mix,&th0g,&thg,&th0l,&thl,&th0C,&thC);
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));

    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
}

//Slot for the pressure envelope calculation for binary mixtures
void FreeFluidsMainWindow::twMixCalcPenvelope(){
    int numPoints=21;
    int i,j;//the loop variable
    double base[numPoints],liquid[numPoints],gas[numPoints],bP[numPoints],dP[numPoints];
    double T;//the working temperature

    T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    for (j=0;j<ui->twMixEnvelope->rowCount();j++) for(i=0;i<ui->twMixEnvelope->columnCount();i++) ui->twMixEnvelope->item(j,i)->setText("");//we clear the content of the results table

    FF_PressureEnvelope(mix,&T,&numPoints,base,bP,gas,dP,liquid);//the calculation
    for(i=0;i<ui->twMixEnvelope->rowCount()-1;i++){
        ui->twMixEnvelope->item(i,0)->setText(QString::number(base[i]));
        ui->twMixEnvelope->item(i,1)->setText(QString::number(bP[i]));
        ui->twMixEnvelope->item(i,2)->setText(QString::number(gas[i]));
        ui->twMixEnvelope->item(i,3)->setText(QString::number(dP[i]));
        ui->twMixEnvelope->item(i,4)->setText(QString::number(liquid[i]));
    }
    ui->twMixEnvelope->item(i,0)->setText(ui->twMixComposition->item(0,1)->text());
    ui->twMixEnvelope->item(i,1)->setText(ui->twMixComposition->item(1,1)->text());
    ui->twMixEnvelope->item(i,2)->setText(QString("P envelope"));
    ui->twMixEnvelope->item(i,3)->setText(QString::number(T));
    ui->twMixEnvelope->item(i,4)->setText(QString("Kelvin"));
}

//Slot for mixture bubble T calculation, and display in table
void FreeFluidsMainWindow::twMixCalcBubbleT(){
    //printf("Act/Eos: %i Mixrule: %i ActModel: %i form: %i 0: %f 1: %f\n",mix->thModelActEos,mix->mixRule,mix->actModel,mix->intForm,mix->intParam[0][1][0],mix->intParam[0][1][1]);
    int i;//the loop variable
    int nPhases=2;
    FF_ThermoProperties th0l,th0g,th0C;//for ideal gas properties
    FF_PhaseThermoProp thl,thg,thC;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    double Z,Zg;
    double bubbleT;
    double bubbleTguess=ui->leMixCalcTempGuess->text().toDouble();

    //clear the content of the results table
    for (i=0;i<ui->twMixCalc->rowCount();i++){
        ui->twMixCalc->item(i,1)->setText("");
        ui->twMixCalc->item(i,2)->setText("");
    }

    //read the selections made for the molar fractions
    for (i=0;i< mix->numSubs;i++){
        thl.c[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }

    //And begin the calculations
    th0l.MW=thl.MW=MW;
    th0l.P=thl.P=th0g.P=thg.P=ui->leMixCalcPres->text().toDouble()*1e5;//we read the selected pressure
    FF_BubbleT(mix,&thl.P,thl.c,&bubbleTguess,&bubbleT,thg.c,thl.subsPhi,thg.subsPhi);
    thg.MW=0;
    for(i=0;i<mix->numSubs;i++)thg.MW=thg.MW+thg.c[i]*mix->baseProp[i].MW;
    th0g.MW=thg.MW;
    th0l.T=thl.T=th0g.T=thg.T=bubbleT;
    thl.fraction=1;
    thg.fraction=0;


    option='l';//determine only the liquid volume
    FF_MixVfromTPeos(mix,&thl.T,&thl.P,thl.c,&option,answerL,answerG,&state);
    //printf("state:%c P:%f Vl:%f Vg:%f\n",state,thl.P,answerL[0],answerG[0]);
    th0l.V=answerL[0];
    thl.V=answerL[0];
    Z=answerL[2];
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);
    phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thl.c,&refT,&refP,&th0l);
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);
    FF_MixThermoEOS(mix,&refT,&refP,&thl);
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);


    option='g';//determine only the gas volume
    FF_MixVfromTPeos(mix,&thg.T,&thg.P,thg.c,&option,answerL,answerG,&state);
    th0g.V=thg.V=answerG[0];
    Zg=answerG[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thg.c,&refT,&refP,&th0g);
    FF_MixThermoEOS(mix,&refT,&refP,&thg);
    phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thl.T,thl.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);
    /*
    */
    writeMixResultsTable(nPhases,mix,&th0g,&thg,&th0l,&thl,&th0C,&thC);
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));

    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
}

//Slot for mixture dew T calculation, and display in table
void FreeFluidsMainWindow::twMixCalcDewT(){
    //printf("Act/Eos: %i Mixrule: %i ActModel: %i form: %i 0: %f 1: %f\n",mix->thModelActEos,mix->mixRule,mix->actModel,mix->intForm,mix->intParam[0][1][0],mix->intParam[0][1][1]);
    int i;//the loop variable
    int nPhases=2;
    FF_ThermoProperties th0l,th0g,th0C;//for ideal gas properties
    FF_PhaseThermoProp thl,thg,thC;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    double Z,Zg;
    double dewT;
    double dewTguess=0;

    //we clear the content of the results table
    for (i=0;i<ui->twMixCalc->rowCount();i++){
        ui->twMixCalc->item(i,1)->setText("");
        ui->twMixCalc->item(i,2)->setText("");
    }

    //Now we need to read the selections made for the molar fractions
    for (i=0;i< mix->numSubs;i++){
        thl.c[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }

    //And begin the calculations
    th0l.MW=thl.MW=MW;
    th0l.P=thl.P=th0g.P=thg.P=ui->leMixCalcPres->text().toDouble()*1e5;//we read the selected pressure
    FF_DewT(mix,&thl.P,thl.c,&dewTguess,&dewT,thg.c,thl.subsPhi,thg.subsPhi);
    thg.MW=0;
    for(i=0;i<mix->numSubs;i++)thg.MW=thg.MW+thg.c[i]*mix->baseProp[i].MW;
    th0g.MW=thg.MW;
    th0l.T=thl.T=th0g.T=thg.T=dewT;
    thl.fraction=1;
    thg.fraction=0;


    option='l';//determine only the liquid volume
    FF_MixVfromTPeos(mix,&thl.T,&thl.P,thl.c,&option,answerL,answerG,&state);
    //printf("state:%c P:%f Vl:%f Vg:%f\n",state,thl.P,answerL[0],answerG[0]);
    th0l.V=answerL[0];
    thl.V=answerL[0];
    Z=answerL[2];
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);
    phiL=exp(answerL[1]+answerL[2]-1)/answerL[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thl.c,&refT,&refP,&th0l);
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);
    FF_MixThermoEOS(mix,&refT,&refP,&thl);
    //printf("state:%c P:%f Vl:%f Pcalc:%f\n",state,thl.P,thl.V,Z*R*thl.T/thl.V);


    option='g';//determine only the gas volume
    FF_MixVfromTPeos(mix,&thg.T,&thg.P,thg.c,&option,answerL,answerG,&state);
    th0g.V=thg.V=answerG[0];
    Zg=answerG[2];
    FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thg.c,&refT,&refP,&th0g);
    FF_MixThermoEOS(mix,&refT,&refP,&thg);
    phiG=exp(answerG[1]+answerG[2]-1)/answerG[2];
    //printf("T:%f P:%f rhoL:%f Vg:%f state:%c phiL:%f phiG:%f\n",thl.T,thl.P,MW/answerL[0]*0.001,answerG[0],state,phiL,phiG);
    /*
    */
    writeMixResultsTable(nPhases,mix,&th0g,&thg,&th0l,&thl,&th0C,&thC);
    ui->twMixCalc->item(5,1)->setText(QString::number(phiG));
    ui->twMixCalc->item(6,1)->setText(QString::number(Zg));

    ui->twMixCalc->item(4,2)->setText(QString::number(phiL));
    ui->twMixCalc->item(6,2)->setText(QString::number(Z));
}

//Slot for the temperature envelope calculation for binary mixtures
void FreeFluidsMainWindow::twMixCalcTenvelope(){
    int numPoints=21;
    int i,j;//the loop variable
    double base[numPoints],liquid[numPoints],gas[numPoints],bT[numPoints],dT[numPoints];
    double P;//the working pressure

    P=ui->leMixCalcPres->text().toDouble()*1e5;//we read the selected pressure
    for (j=0;j<ui->twMixEnvelope->rowCount();j++) for(i=0;i<ui->twMixEnvelope->columnCount();i++) ui->twMixEnvelope->item(j,i)->setText("");//we clear the content of the results table

    FF_TemperatureEnvelope(mix,&P,&numPoints,base,bT,gas,dT,liquid);//the calculation
    for(i=0;i<ui->twMixEnvelope->rowCount()-1;i++){
        ui->twMixEnvelope->item(i,0)->setText(QString::number(base[i]));
        ui->twMixEnvelope->item(i,1)->setText(QString::number(bT[i]));
        ui->twMixEnvelope->item(i,2)->setText(QString::number(gas[i]));
        ui->twMixEnvelope->item(i,3)->setText(QString::number(dT[i]));
        ui->twMixEnvelope->item(i,4)->setText(QString::number(liquid[i]));
    }
    ui->twMixEnvelope->item(i,0)->setText(ui->twMixComposition->item(0,1)->text());
    ui->twMixEnvelope->item(i,1)->setText(ui->twMixComposition->item(1,1)->text());
    ui->twMixEnvelope->item(i,2)->setText(QString("T envelope"));
    ui->twMixEnvelope->item(i,3)->setText(QString::number(P*1e-5));
    ui->twMixEnvelope->item(i,4)->setText(QString("bara"));
}

//Slot for mixture VL flash P,T calculation, and display in table
void FreeFluidsMainWindow::twMixCalc2PhFlashPT(){
    FF_FeedData data;
    //printf("Act/Eos: %i Mixrule: %i ActModel: %i form: %i 0: %f 1: %f\n",mix->thModelActEos,mix->mixRule,mix->actModel,mix->intForm,mix->intParam[0][1][0],mix->intParam[0][1][1]);
    int i;//the loop variable
    int nPhases=2;
    FF_ThermoProperties th0A,th0B,th0C;//for ideal gas properties. C will not be used
    FF_PhaseThermoProp thA,thB,thC;//here will be stored the result of the calculations for the gas and liquid phases
    double c[mix->numSubs];//feed concentration
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiA,phiB;
    double Za,Zb;
    double Gr;//Modified reduced gibbs energy

    //we clear the content of the results table
    for (i=0;i<ui->twMixCalc->rowCount();i++){
        ui->twMixCalc->item(i,1)->setText("");
        ui->twMixCalc->item(i,2)->setText("");
        ui->twMixCalc->item(i,3)->setText("");
    }

    //Now we need to read the selections made for the molar fractions
    for (i=0;i< mix->numSubs;i++){
        c[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
        data.z[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }

    //And begin the calculations
    //th0B.MW=thB.MW=MW;
    th0B.T=thB.T=th0A.T=thA.T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    th0B.P=thB.P=th0A.P=thA.P=1e5*ui->leMixCalcPres->text().toDouble();//we read the selected pressure
    data.mix=mix;
    data.P=thB.P;
    data.T=thB.T;

    if(ui->rbMixCalcStd->isChecked()) FF_TwoPhasesFlashPT(mix,&thB.T,&thB.P,c,thB.c,thA.c,thB.subsPhi,thA.subsPhi,&thA.fraction);
    else if(ui->rbMixCalcGlobalOptSA->isChecked()) FF_TwoPhasesFlashPTSA(&data,thB.c,thA.c,thB.subsPhi,thA.subsPhi,&thA.fraction,&Gr);
    else if(ui->rbMixCalcGlobalOptDE->isChecked()) FF_TwoPhasesFlashPTDE(&data,thB.c,thA.c,thB.subsPhi,thA.subsPhi,&thA.fraction,&Gr);

    thB.fraction=1-thA.fraction;
    thB.MW=0;
    thA.MW=0;
    for(i=0;i<mix->numSubs;i++){
        thB.MW=thB.MW+thB.c[i]*mix->baseProp[i].MW;
        thA.MW=thA.MW+thA.c[i]*mix->baseProp[i].MW;
    }
    th0B.MW=thB.MW;
    th0A.MW=thA.MW;
    if(mix->thModelActEos==1){//if phi-phi used, use eos for the expected liquid phase
        option='s';
        FF_MixVfromTPeos(mix,&thB.T,&thB.P,thB.c,&option,answerL,answerG,&state);
        if((state=='L')||(state=='l')||(state=='U')||(state=='u')){
            th0B.V=thB.V=answerL[0];
            Zb=answerL[2];
            phiB=exp(answerL[1]+answerL[2]-1)/answerL[2];
        }
        else if((state=='G')||(state=='g')){
            th0B.V=thB.V=answerG[0];
            Zb=answerG[2];
            phiB=exp(answerG[1]+answerG[2]-1)/answerG[2];
        }
        ui->twMixCalc->item(6,2)->setText(QString::number(Zb));
        FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thB.c,&refT,&refP,&th0B);
        FF_MixThermoEOS(mix,&refT,&refP,&thB);
    }
    if(mix->thModelActEos!=2){//If not LLE gamma-gamma used, use eos for the expected gas phase
        option='s';
        FF_MixVfromTPeos(mix,&thA.T,&thA.P,thA.c,&option,answerL,answerG,&state);
        if((state=='L')||(state=='l')||(state=='U')||(state=='u')){
            th0A.V=thA.V=answerL[0];
            Za=answerL[2];
            phiA=exp(answerL[1]+answerL[2]-1)/answerL[2];
        }
        else if((state=='G')||(state=='g')){
            th0A.V=thA.V=answerG[0];
            Za=answerG[2];
            phiA=exp(answerG[1]+answerG[2]-1)/answerG[2];
        }
        ui->twMixCalc->item(6,1)->setText(QString::number(Za));
        FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thA.c,&refT,&refP,&th0A);
        FF_MixThermoEOS(mix,&refT,&refP,&thA);
    }
    if(ui->rbMixCalcStd->isChecked()){
        Gr=0;
        for(i=0;i<mix->numSubs;i++){
            Gr=Gr+(1-thA.fraction)*thB.c[i]*(log(thB.c[i]*thB.subsPhi[i]))+thA.fraction*thA.c[i]*(log(thA.c[i]*thA.subsPhi[i]));
        }
    }
    writeMixResultsTable(nPhases,mix,&th0A,&thA,&th0B,&thB,&th0C,&thC);
    ui->leMixCalcGibbs->setText(QString::number(Gr));
    if(mix->thModelActEos!=1){
        FF_SubsActivityData actData[mix->numSubs];
        FF_ExcessData excData;
        //FF_Activity(mix,&thB.T,thB.c,actData);
        FF_ActivityDerivatives(&mix->actModel,&mix->numSubs,mix->baseProp,mix->intParam,&mix->intForm,&thB.T,thB.c,actData,&excData);
        double gE=0;
        double hE=-thB.T*excData.dgE_dT;
        for (i=0;i<mix->numSubs;i++){
            gE=gE+thB.c[i]*(actData[i].lnGammaC+actData[i].lnGammaR+actData[i].lnGammaSG);
        }
        ui->twMixCalc->item(74,2)->setText(QString::number(gE));
        ui->twMixCalc->item(75,2)->setText(QString::number(hE));
    }
}

//Slot for mixture three phases P,T flash calculation, and display in table
void FreeFluidsMainWindow::twMixCalc3PhFlashPT(){
    //FF_VLflashPTGO();
    FF_FeedData data;
    //printf("Act/Eos: %i Mixrule: %i ActModel: %i form: %i 0: %f 1: %f\n",mix->thModelActEos,mix->mixRule,mix->actModel,mix->intForm,mix->intParam[0][1][0],mix->intParam[0][1][1]);
    int i;//the loop variable
    int nPhases=3;
    FF_ThermoProperties th0A,th0B,th0C;//for ideal gas properties
    FF_PhaseThermoProp thA,thB,thC;//here will be stored the result of the calculations for the liquid and gas phases
    double refT=298.15;//reference temperature for thermodynamic properties (as ideal gas)
    double refP=1.01325e5;//reference pressure
    char option='s';//for asking for both states (liquid and gas) calculation, and state determination
    char state;//we will receive here the state of the calculation
    double MW=ui->leMixCalcMWmix->text().toDouble();
    double answerL[3],answerG[3];
    double phiL,phiG;
    double Za,Zb,Zc;
    double Gr;//Modified reduced gibbs energy

    //we clear the content of the results table
    for (i=0;i<ui->twMixCalc->rowCount();i++){
        ui->twMixCalc->item(i,1)->setText("");
        ui->twMixCalc->item(i,2)->setText("");
        ui->twMixCalc->item(i,3)->setText("");
    }

    //Now we need to read the selections made for the molar fractions
    for (i=0;i< mix->numSubs;i++){
        data.z[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }

    //And begin the calculations
    th0A.MW=thA.MW=MW;
    th0A.T=thA.T=th0B.T=thB.T=th0C.T=thC.T=273.15+ui->leMixCalcTemp->text().toDouble();//we read the selected temperature
    th0A.P=thA.P=th0B.P=thB.P=th0C.P=thC.P=1e5*ui->leMixCalcPres->text().toDouble();//we read the selected pressure
    data.mix=mix;
    data.P=thA.P;
    data.T=thA.T;

    FF_ThreePhasesFlashPTSA(&data,thA.c,thB.c,thC.c,thA.subsPhi,thB.subsPhi,thC.subsPhi,&thA.fraction,&thB.fraction,&Gr);
    thC.fraction=1-thA.fraction-thB.fraction;
    thA.MW=0;
    thB.MW=0;
    thC.MW=0;
    for(i=0;i<mix->numSubs;i++){
        thA.MW=thA.MW+thA.c[i]*mix->baseProp[i].MW;
        thB.MW=thB.MW+thB.c[i]*mix->baseProp[i].MW;
        thC.MW=thC.MW+thC.c[i]*mix->baseProp[i].MW;
    }
    th0A.MW=thA.MW;
    th0B.MW=thB.MW;
    th0C.MW=thC.MW;

    if(mix->thModelActEos==1){//if phi-phi used, use eos for the expected liquid phases
        option='s';
        FF_MixVfromTPeos(mix,&thB.T,&thB.P,thB.c,&option,answerL,answerG,&state);
        if((state=='L')||(state=='l')||(state=='U')||(state=='u')){
            th0B.V=thB.V=answerL[0];
            Zb=answerL[2];
            //phiB=exp(answerL[1]+answerL[2]-1)/answerL[2];
        }
        else if((state=='G')||(state=='g')){
            th0B.V=thB.V=answerG[0];
            Zb=answerG[2];
            //phiB=exp(answerG[1]+answerG[2]-1)/answerG[2];
        }
        ui->twMixCalc->item(6,2)->setText(QString::number(Zb));
        FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thB.c,&refT,&refP,&th0B);
        FF_MixThermoEOS(mix,&refT,&refP,&thB);

        FF_MixVfromTPeos(mix,&thC.T,&thC.P,thC.c,&option,answerL,answerG,&state);
        if((state=='L')||(state=='l')||(state=='U')||(state=='u')){
            th0C.V=thC.V=answerL[0];
            Zc=answerL[2];
            //phiC=exp(answerL[1]+answerL[2]-1)/answerL[2];
        }
        else if((state=='G')||(state=='g')){
            th0C.V=thC.V=answerG[0];
            Zc=answerG[2];
            //phiC=exp(answerG[1]+answerG[2]-1)/answerG[2];
        }
        ui->twMixCalc->item(6,3)->setText(QString::number(Zc));
        FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thC.c,&refT,&refP,&th0C);
        FF_MixThermoEOS(mix,&refT,&refP,&thC);
    }

    if(mix->thModelActEos!=2){//If not LLE gamma-gamma used, use eos for the expected gas phase
        option='s';
        FF_MixVfromTPeos(mix,&thA.T,&thA.P,thA.c,&option,answerL,answerG,&state);
        if((state=='L')||(state=='l')||(state=='U')||(state=='u')){
            th0A.V=thA.V=answerL[0];
            Za=answerL[2];
            //phiA=exp(answerL[1]+answerL[2]-1)/answerL[2];
        }
        else if((state=='G')||(state=='g')){
            th0A.V=thA.V=answerG[0];
            Za=answerG[2];
            //phiA=exp(answerG[1]+answerG[2]-1)/answerG[2];
        }
        ui->twMixCalc->item(6,1)->setText(QString::number(Za));
        FF_MixIdealThermoEOS(&mix->numSubs,mix->cp0Corr,thA.c,&refT,&refP,&th0A);
        FF_MixThermoEOS(mix,&refT,&refP,&thA);
    }
    if(ui->rbMixCalcStd->isChecked()){
        Gr=0;
        for(i=0;i<mix->numSubs;i++){
            Gr=Gr+thA.fraction*thA.c[i]*(log(thA.c[i]*thA.subsPhi[i]))+thB.fraction*thB.c[i]*(log(thB.c[i]*thB.subsPhi[i]))+
                    thC.fraction*thC.c[i]*(log(thC.c[i]*thC.subsPhi[i]));
        }
    }
    writeMixResultsTable(nPhases,mix,&th0A,&thA,&th0B,&thB,&th0C,&thC);
    ui->leMixCalcGibbs->setText(QString::number(Gr));

}

//Slot for checking stability of a composition
void FreeFluidsMainWindow::mixCalcStabCheck(){
    FF_FeedData data;
    int i;
    double tpd,tpdX[15];
    data.mix=mix;
    data.P=1e5*ui->leMixCalcPres->text().toDouble();
    data.T=273.15+ui->leMixCalcTemp->text().toDouble();
    //Now we need to read the selections made for the molar fractions
    for (i=0;i< mix->numSubs;i++){
        data.z[i]=ui->twMixComposition->item(i,5)->text().toDouble();//substance molar fraction
    }
    if(ui->rbMixCalcGlobalOptSA->isChecked()) FF_StabilityCheckSA(&data,&tpd,tpdX);
    else FF_StabilityCheck(&data,&tpd,tpdX);

    ui->leMixCalcStabResult->setText(QString::number(tpd));

}

//Slot for checking stability of the results obtained
void FreeFluidsMainWindow::mixResStabCheck(){
    FF_FeedData data;
    int i;
    double tpd,tpdX[15];
    data.mix=mix;
    data.P=1e5*ui->leMixCalcPres->text().toDouble();
    data.T=273.15+ui->leMixCalcTemp->text().toDouble();
    //Now we need to read the selections made for the molar fractions
    if(ui->rbMixResPh1->isChecked()){
        for (i=0;i< mix->numSubs;i++){
            data.z[i]=ui->twMixCalc->item(29+i,1)->text().toDouble();//substance molar fraction
        }
    }
    else if(ui->rbMixResPh2->isChecked()){
        for (i=0;i< mix->numSubs;i++){
            data.z[i]=ui->twMixCalc->item(29+i,2)->text().toDouble();//substance molar fraction
        }
    }
    else if(ui->rbMixResPh3->isChecked()){
        for (i=0;i< mix->numSubs;i++){
            data.z[i]=ui->twMixCalc->item(29+i,3)->text().toDouble();//substance molar fraction
        }
    }
    if(ui->rbMixCalcGlobalOptSA->isChecked()) FF_StabilityCheckSA(&data,&tpd,tpdX);
    else FF_StabilityCheck(&data,&tpd,tpdX);
    ui->leMixResStabResult->setText(QString::number(tpd));

}

//Slot for calculating transport properties
void FreeFluidsMainWindow::mixResCalcTransport(){
    int i,j;
    double c[mix->numSubs],P,T,V,visc,thCond,surfTens;//molar concentrations
    if(ui->rbMixResPh1->isChecked()) j=1;
    else if(ui->rbMixResPh2->isChecked()) j=2;
    else if(ui->rbMixResPh3->isChecked()) j=3;
    for (i=0;i<mix->numSubs;i++) c[i]=ui->twMixCalc->item(29+i,j)->text().toDouble();//substance molar fraction
    P=1e5*ui->leMixCalcPres->text().toDouble();
    T=273.15+ui->leMixCalcTemp->text().toDouble();
    if(ui->rbMixResAsLiq->isChecked()){
        int useParachor=1;
        double rhoL,rhoG,y[mix->numSubs];
        if(j==2){
            rhoL=1e6/ui->twMixCalc->item(7,2)->text().toDouble();
            rhoG=1e6/ui->twMixCalc->item(7,1)->text().toDouble();
            for (i=0;i<mix->numSubs;i++) y[i]=ui->twMixCalc->item(29+i,1)->text().toDouble();
        }
        else{
            rhoL=1e6/ui->twMixCalc->item(7,1)->text().toDouble();
            rhoG=1e6/ui->twMixCalc->item(7,2)->text().toDouble();
            for (i=0;i<mix->numSubs;i++) y[i]=ui->twMixCalc->item(29+i,2)->text().toDouble();
        }
        FF_MixLiqViscTeja(mix,&T,&P,c,&visc);
        ui->twMixCalc->item(77,j)->setText(QString::number(visc));
        FF_MixLiqThCondLi(mix,&T,&P,c,&thCond);
        ui->twMixCalc->item(78,j)->setText(QString::number(thCond));
        for (i=0;i<mix->numSubs;i++){
            if(!(mix->baseProp[i].Pa>0)) useParachor=0;
        }
        //FF_MixLiqSurfTensLinear(mix,&T,&P,c,&surfTens);
        if(useParachor==0) FF_MixLiqSurfTensWinterfeld(mix,&T,&P,c,&surfTens);
        else FF_MixLiqSurfTensMcLeod(mix,&rhoL,&rhoG,c,y,&surfTens);
        ui->twMixCalc->item(79,j)->setText(QString::number(surfTens));
    }
    else{
        //FF_MixGasViscTPcpWilke(mix,&T,&P,c,&visc);
        FF_MixGasViscTPcpLucas(mix,&T,&P,c,&visc);
        ui->twMixCalc->item(77,j)->setText(QString::number(visc));
        FF_MixLpGasThCondTpMason(mix,&T,c,&thCond);
        ui->twMixCalc->item(78,j)->setText(QString::number(thCond));
        ui->twMixCalc->item(79,j)->setText("");
    }

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


