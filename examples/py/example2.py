#-*- coding: utf-8 -*-


"""
Get the ODB attributes and create DCA files 

"""
import os
from datetime import datetime 

# From  utils
from   odb4py.utils import OdbObject

# From  core module
from   odb4py.core  import odb_open , odb_dca


# Start
start  = datetime.now()

# Ncpu for DCA creation 
NCPU = 4

# Path
exo_path   ="../odb_samples/metcoop"
dbtype = "CCMA"
dbpath = "/".join(( exo_path  , dbtype))


# Get the attributes and use the available tables in pool(s)
db = OdbObject(dbpath)
db_attr  =db.get_attrib()
db_tables=db_attr["tables"]

# Connect 
conn= odb_open (dbpath )

# Check if the files existe  (Maybe the ODB has been opened before)
if not os.path.isdir (exo_path+"/dca" ):
   ic =odb_dca (database  = dbpath,
                dbtype    = dbtype,
                ncpu      = NCPU ,       # Number of CPUs for parallel run
                extra_args=" -u ",       # -u means : Update DCA files if found
                tables    =  db_tables,   # Use the one found from attributes
                verbose   =True 
                )

   if ic < 0 :
      print("Failed to create DCA files")
      sys.exist()

# odb_close function is an object of conn 
conn.odb_close()


# End
end  = datetime.now()
duration = end -  start
print("Runtime duration:" , duration  )
