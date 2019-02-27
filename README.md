# FreeFluidsGui
FreeFluidsGui is a graphical user interface to the FreeFluidsC soft.
 It has been constructed using Qt Creator and you will find the source soft in the include and src folders.
 If you want to compile the application you will need, in addition to Qt Creator, the sources and headers from the FreeFluidsC soft and,
 as FreeFluidsC uses Nlopt for optimization, the Nlopt linking library. At runtime you will need the DLLs for Nlopt, Qt, and the C compiler you use.
 For the more normal user I have included a compilation of the soft in Windows, using MinGW, the result is in the windowsexe folder.
 In this folder you will find the application, the needed DLLs, and a database in MS Access and SqLite formats.
 The database is strictly necessary, as the soft will take the products data from this database.
 The Access database is accessed via ODBC, so, if you want to use it you will need to register it as an ODBC source.
 In case of impossibility to connect to the Access database a connection is established with the SqLite database.
