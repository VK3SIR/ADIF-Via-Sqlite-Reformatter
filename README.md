#Sample Application: ADIF Converter
 
Many of us used DXLab dxkeeper ADIF logs for our wsjtx_log.adi files. [ With JTDX, data is scanned and made available for us to use with contacts that we have already made connection with; it assists with future logging. ].
 
But too much data is actually returned in the produced ADIF file.

##Whar is this App About?

This brief sample app, which was used as a demo piece for Using SQLite, UI blocking and overcoming UI blocking issues, may be of use to some in our community ...
 
It demonstrates using Qt's inbuilt SQLITE support as well as its extremely powerful in-memory-data-collection-managment functions i.e. records are transferred from a source ADIF file, "pruned of fields" into a SQLITE database, and then the database can be written back to a clean ADIF file.
 
It was also used as an example for AI refinement of complex algorithms ... and to demonstrate pitfalls (i.e. one MUST have a good knowledge of what the AI is doing before one uses it).
 
It is not perfect and has some fleas ... Yet it is made available here to our community:

##Base Reference

See: https://sourceforge.net/projects/hamlib-sdk/files/Programming/Qt/AdifToSqlite-Example.7z as master