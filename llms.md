# odb4py - High-Performance ODB1 Access for Python

> Python interface to access and query ECMWF ODB1 (Observation DataBase) using a native C backend.

## Technical Summary
- **Core:** Pure C implementation (pruned from ECMWF ODB_API 0.18.1).
- **Type:** High-performance wrapper for ODB1 (CMA) databases.
- **Independence:** No system-level ECMWF installation required (self-contained).
- **Interoperability:** Native integration with NumPy and Pandas. Export to NetCDF and SQLite.

## Key API Components
- `odb4py.core.odb_open`: Open connection to ODB.
- `odb4py.core.odb_dict`: Execute ODB-SQL and fetch data as a Python dictionary.
- `odb4py.utils.SqlParser`: Tooling to clean and validate ODB-SQL queries.
- `odb4py.utils.OdbObject`: Introspection of ODB attributes and tables.

## Minimal Example (The "Quick Start" for LLMs)
```python
from odb4py.core import odb_open
from odb4py.utils import SqlParser

# 1. Connect and query
conn = odb_open(database='/path/to/ODB')
query = "SELECT statid, lat, lon, obsvalue FROM hdr, body"

# 2. Fetch data (handling SQL cleaning automatically)
p = SqlParser()
data = conn.odb_dict(
    database='/path/to/ODB',
    sql_query=p.clean_string(query),
    nfunc=p.get_nfunc(query)
)
print(data)

DCA Management     : Use odb_dca to index pools in parallel (ncpu=4).
Parallel Processing: Supports poolmask to target specific ODB subsets.
Conversion         : Built-in methods for NetCDF & SQLite conversion.
