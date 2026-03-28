import sys  
import numpy as np  

sys.path.insert( 0, "/home/micro/Bureau/rmib_dev/github/odb4py_1.3.1" ) 

from  utils      import  OdbObject  , SqlParser , OdbEnv 


dbpath = "/home/micro/Bureau/rmib_dev/github/odb4py_1.3.1/examples/odb_samples/chmi/CCMA"
db      = OdbObject ( dbpath )
db_attr = db.get_attrib()
db_type = db_attr["type"]
db_name = db_attr["name"]
tabs    = db_attr["tables"]



print( db_attr) 
quit()

env= OdbEnv()

env.update_var ( {"ODB_IO_METHOD":"4"}  )
env.update_var ( {"ODB_GRPSIZE":"46212154"}  )


v  =env.get_var( "ODB_IO_METHOD" )


vars=  env.dump_vars()
print(vars ) 


quit()
from  core       import  odb_open   , odb_gcdist , odb_geopoints 
from  info       import  odb_tables   , odb_varno  , odb_functions
from  convert    import  odb2nc



dbpath="/hpcperm/cvah/rmib_dev/odb4py_1.3.1/examples/odb_samples/rmi/ECMA.synop"

sql ="select degrees(lat) ,degrees(lon) , obsvalue from hdr , body"

db      = OdbObject ( dbpath )
db_attr = db.get_attrib()
db_type = db_attr["type"]
db_name = db_attr["name"]
tabs    = db_attr["tables"]


env= OdbEnv()  

print( env )

quit()
conn=odb_open( database= dbpath  )
ic =conn.odb_dca ( database= dbpath, dbtype=db_type, ncpu=4 , verbose =True , tables= tabs  )


qfile="/hpcperm/cvah/rmib_dev/odb4py_1.3.1/examples/sql/ccma_view.sql"                             


"""sql="SELECT obstype@hdr,codetype@hdr,statid,varno,lat@hdr,lon@hdr,vertco_type@body,vertco_reference_1@body,\
            sensor@hdr,date,time@hdr,report_status.active@hdr,report_status.blacklisted@hdr,\
            report_status.passive@hdr,report_status.rejected@hdr,datum_status.active@body,\
            datum_status.blacklisted@body,datum_status.passive@body,datum_status.rejected@body,\
            datum_anflag.final,an_depar,fg_depar,obsvalue,final_obs_error@errstat,biascorr_fg,lsm@modsurf \
            FROM hdr,body,modsurf,errstat WHERE ( obstype@hdr = $obstype ) AND ( varno@body = $varno ) AND ( codetype@hdr >= $subtype1 )\
            AND ( codetype@hdr <= $subtype2 ) AND ( vertco_reference_1@body > $press1 ) \
            AND ( vertco_reference_1@body <= $press2 ) AND ( an_depar IS NOT NULL)"""

sql="SELECT obstype@hdr,codetype@hdr,statid,varno, degrees(lat@hdr) ,degrees(lon@hdr) ,vertco_type@body,vertco_reference_1@body FROM  hdr ,  body "

p  = SqlParser()
nf = p.get_nfunc ( sql  )


tabs  =odb_tables()
varno =odb_varno()
funcs =odb_functions()


for t in tabs :
    print( t )

for k,v in varno.items():
    print(k,  v)  

for f in funcs:
    print( f) 

quit()

#data= conn.odb_dict ( database=dbpath , sql_query=sql   , queryfile = qfile  , nfunc= nf , pbar =True , verbose = True )
#data= conn.odb_array( database=dbpath , sql_query=None   , queryfile = qfile  , pbar =True , verbose = True )
#data =conn.odb_geopoints(database  = dbpath,  condition = None, unit="degrees", verbose =False  )
#dist= odb_gcdist(lon1, lat1 ,lon2 , lat2   )
#odb2nc( database = dbpath ,  sql_query=sql , nfunc=nf , ncfile ="output.nc", pbar=True  , verbose =False  )




"""lat1=np.array(data['degrees(lat)'])
lon1=np.array(data['degrees(lon)']) 

lat2 = lat1
lon2 = lon1


dist= odb_gcdist(lon1, lat1 ,lon2 , lat2   )
for i in range(dist.shape[0]):
    for j in range(dist.shape[1]):
        print( dist[i , j]  )"""

conn.odb_close()
quit()

