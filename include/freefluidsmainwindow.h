/*
 * freefluidsmainwindow.h
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

#ifndef FREEFLUIDSMAINWINDOW_H
#define FREEFLUIDSMAINWINDOW_H

//#include <string>
#include <QMainWindow>
#include <QtGui>
#include <QtWidgets>
#include <QtSql/QSqlDatabase>
#include <QtSql>

#include "FFbasic.h"
#include "FFeosPure.h"
#include "FFeosMix.h"
#include "FFphysprop.h"
#include "FFtools.h"
#include "FFbaseClasses.h"
#include "databasetools.h"


namespace Ui {
class FreeFluidsMainWindow;
}

class FreeFluidsMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FreeFluidsMainWindow(QWidget *parent = 0);
    ~FreeFluidsMainWindow();

private slots:
    void cbSubsCalcSelLoad(int);//Slot for loading the existing EOS and Cp0 correlations for the selected substance
    void cbSubsCalcEosUpdate(int position);//Slot for eos data update
    void cbSubsCalcCp0Update(int position);//Slot for cp0 data update
    void twSubsCalcUpdate();//Slot for substance calculation, and display in table
    void btnSubsCalcAltCalc();//Slot for alternative calculation (no from T andP)
    void twSubsCalcExport();//Slot for table content exportation in csv ; delimited format
    void btnSubsCalcExportSubs();//Slot for substance exportation in binary format
    void btnSubsCalcTransfer();//Slot for transfer from eos calculation to correlation calculation data tables
    void cbSubsToolsCorrLoad(int position);//Slot for correlation selection load on substance and screen
    void btnSubsToolsFillTable();//Slot for filling table with data from correlations
    void twSubsToolsImport();//Slot for table content importation from csv ; delimited format
    void cbSubsToolsSelOptCorr(int position);//Slot for fixing the the bounds of the needed parameters
    void btnSubsToolsClearCoef();//Slot for clearing content of the forced coeffients table, and optimization errors
    void btnSubsToolsFindCorr();//Slot for correlation coefficients calculation
    void btnSubsToolsFindEOS();//Slot for EOS coefficients calculation
    void btnSubsToolsTestEOS();//Slot for EOS coefficients test
    void twSubsToolsClear();//Slot for clearing content in the table
    void twSubsToolsInterchange();//Slot for interchanging table columns
    void twSubsToolsDoOperation();//Slot for do operation on a data column
    void btnSubsToolAddEos();//Slot for adding EOS to database
    void btnSubsToolAddCorr();//Slot for adding correlation to database
    void twMixCompositionClear();//Slot for clearing content in the table
    void twMixCompositionAdd();//Slot for adding a substance to the table
    void twMixCompositionCalcFract();//Slot for calculation of mass and molar fractions
    void cbMixCalcEosTypeLoad(int);//Slot for loading the existing EOS and Cp0 correlations for the selected substances
    void cbMixCalcMixRuleLoad(int);//Slot for storing the selected mixing rule
    void twMixIntParamEosDisplay(int row, int column);//Slot for displaying the intParam array for eos, for the selecte pair
    void twMixIntParamEosFill(int row);//Slot for filling the intParam array for eos, from the database
    void twMixIntParamEosUpdate(int row, int column);//Slot for updating the intParam array for eos, from the display
    void twMixCalcUpdate();//Slot for mixture calculation, and display in table
    void twMixCalcBubbleT();//Slot for mixture bubble T calculation, and display in table
    void twMixCalcDewT();//Slot for mixture dew T calculation, and display in table
    void twMixCalcBubbleP();//Slot for mixture bubble P calculation, and display in table
    void twMixCalcDewP();//Slot for mixture dew P calculation, and display in table
    void twMixCalcPenvelope();//Slot for the pressure envelope calculation for binary mixtures
    void on_actionDisplay_license_triggered();

private:
    Ui::FreeFluidsMainWindow *ui;

private:
    //general usage
    QSqlDatabase db;
    QSqlQueryModel *subsListModel;
    QDoubleValidator *presBarValidator;
    QDoubleValidator *tempCValidator;

    //Substance calculation usage
    FF::Substance *subs;//To hold substance information, for substance calculation or addition to mixture
    FF_SubstanceData *subsData;
    QSqlQueryModel *subsCalcEOSModel;
    QTableView *tvSubsCalcSelEOS;
    QSqlQueryModel *subsCalcCp0Model;
    QTableView *tvSubsCalcSelCp0;
    QCompleter *subsCompleter;

    //Subst tools usage
    QSqlQueryModel *subsToolsCorrModel;
    QTableView *tvSubsToolsSelCorr;
    //QSqlTableModel *tableModel;
    //QDataWidgetMapper *mapper;

    //Mixture calculation usage
    FF::ThermoSystem *thSys;
    QSqlQueryModel *mixCalcEOSModel[6];
    QTableView *tvMixCalcSelEOS[6];
    QSqlQueryModel *mixCalcCp0Model[6];
    QTableView *tvMixCalcSelCp0[6];
    int numSubs;//number of substances really used in the calculation of mixtures
    double *c;//the concentration of substances in the mixture
    //QString eosType;
    enum FF_MixingRule rule;//the mixing rule to use
    float pintParamEos[12][12][6];//Will hold the eos interaction parameters to be used for each pair of substances
    float pintParamAct[12][12][6];//Will hold the activity interaction parameters to be used for each pair of substances
    QSqlQueryModel *mixIntParamSelModel;//model for display the interaction parameters available for the pair, in the combobox
    QTableView *tvMixIntParamSel;//table for display the interaction parameters available for the pair

    int eosSel[12],cp0Sel[12];//the number of row selected(in the combobox) for eos and cp0 correlation, for each possible substance
    void getMixEosCpSel();//Pass the number of the rows selected for eos, and cp0 correlation, for each possible substance, to an array format
};

#endif // FREEFLUIDSMAINWINDOW_H
