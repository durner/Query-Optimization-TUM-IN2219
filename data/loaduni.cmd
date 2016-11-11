@echo off
set admin=..\bin\admin
%admin% initdb uni
%admin% createtable uni professoren persnr key int name string rang string raum int
%admin% createtable uni studenten matrnr key int name string semester int
%admin% createtable uni vorlesungen vorlnr key int titel string sws int gelesenvon int
%admin% createtable uni voraussetzen vorgaenger int nachfolger int
%admin% createtable uni hoeren matrnr int vorlnr int
%admin% createtable uni assistenten persnr key int name string fachgebiet string boss int
%admin% createtable uni pruefen matrnr int vorlnr int persnr int note int
%admin% insertvalues uni professoren 2125 Sokrates C4 226
%admin% insertvalues uni professoren 2126 Russel C4 232
%admin% insertvalues uni professoren 2127 Kopernikus C3 310
%admin% insertvalues uni professoren 2133 Popper C3 52
%admin% insertvalues uni professoren 2134 Augustinus C3 309
%admin% insertvalues uni professoren 2136 Curie C4 36
%admin% insertvalues uni professoren 2137 Kant C4 7
%admin% insertvalues uni studenten 24002 Xenokrates 18
%admin% insertvalues uni studenten 25403 Jonas 12
%admin% insertvalues uni studenten 26120 Fichte 10
%admin% insertvalues uni studenten 26830 Aristoxenos 8
%admin% insertvalues uni studenten 27550 Schopenhauer 6
%admin% insertvalues uni studenten 28106 Carnap 3
%admin% insertvalues uni studenten 29120 Theophrastos 2
%admin% insertvalues uni studenten 29555 Feuerbach 2
%admin% insertvalues uni vorlesungen 5001 Grundzüge 4 2137
%admin% insertvalues uni vorlesungen 5041 Ethik 4 2125
%admin% insertvalues uni vorlesungen 5043 Erkenntnistheorie 3 2126
%admin% insertvalues uni vorlesungen 5049 Mäeutik 2 2125
%admin% insertvalues uni vorlesungen 4052 Logik 4 2125
%admin% insertvalues uni vorlesungen 5052 Wissenschaftstheorie 3 2126
%admin% insertvalues uni vorlesungen 5216 Bioethik 2 2126
%admin% insertvalues uni vorlesungen 5259 "Der Wiener Kreis" 2 2133
%admin% insertvalues uni vorlesungen 5022 "Glaube und Wissen" 2 2134
%admin% insertvalues uni vorlesungen 4630 "Die 3 Kritiken" 4 2137
%admin% insertvalues uni voraussetzen 5001 5041
%admin% insertvalues uni voraussetzen 5001 5043
%admin% insertvalues uni voraussetzen 5001 5049
%admin% insertvalues uni voraussetzen 5041 5216
%admin% insertvalues uni voraussetzen 5043 5052
%admin% insertvalues uni voraussetzen 5041 5052
%admin% insertvalues uni voraussetzen 5052 5259
%admin% insertvalues uni hoeren 26120 5001
%admin% insertvalues uni hoeren 27550 5001
%admin% insertvalues uni hoeren 27550 4052
%admin% insertvalues uni hoeren 28106 5041
%admin% insertvalues uni hoeren 28106 5052
%admin% insertvalues uni hoeren 28106 5216
%admin% insertvalues uni hoeren 28106 5259
%admin% insertvalues uni hoeren 29120 5001
%admin% insertvalues uni hoeren 29120 5041
%admin% insertvalues uni hoeren 29120 5049
%admin% insertvalues uni hoeren 29555 5022
%admin% insertvalues uni hoeren 25403 5022
%admin% insertvalues uni hoeren 29555 5001
%admin% insertvalues uni assistenten 3002 Platon Ideenlehre 2125
%admin% insertvalues uni assistenten 3003 Aristoteles Syllogistik 2125
%admin% insertvalues uni assistenten 3004 Wittgenstein Sprachtheorie 2126
%admin% insertvalues uni assistenten 3005 Rhetikus Planetenbewegung 2127
%admin% insertvalues uni assistenten 3006 Newton "Keplersche Gesetze" 2127
%admin% insertvalues uni assistenten 3007 Spinoza "Gott und Natur" 2134
%admin% insertvalues uni pruefen 28106 5001 2126 1
%admin% insertvalues uni pruefen 25403 5041 2125 2
%admin% insertvalues uni pruefen 27550 4630 2137 2
