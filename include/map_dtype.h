/*
 * These lines have been taken from  :   include/privpub.h  
 * 
 * Shows the different datatypes and their size ! 
#define DATATYPE_UNDEF                0x0           (0 dec)   undef 
#define DATATYPE_BIT                  0x100         (256 dec) bit   : 1 bits, 0 bytes 
#define DATATYPE_INT1                 0x10f         (271 dec) char  : 8 bits, 1 bytes 
#define DATATYPE_INT2                 0x113         (275 dec) short : 16 bits, 2 bytes 
#define DATATYPE_INT4                 0x117         (279 dec) int   : 32 bits, 4 bytes 
#define DATATYPE_INT8                 0x11b         (283 dec) long long : 64 bits, 8 bytes 
#define DATATYPE_UINT1                0x10e         (270 dec) uchar : 8 bits, 1 bytes 
#define DATATYPE_UINT2                0x112         (274 dec) ushort : 16 bits, 2 bytes 
#define DATATYPE_UINT4                0x116         (278 dec) uint   : 32 bits, 4 bytes 
#define DATATYPE_UINT8                0x11a         (282 dec) ulonglong : 64 bits, 8 bytes 
#define DATATYPE_REAL4                0x216         (534 dec) float  : 32 bits, 4 bytes 
#define DATATYPE_REAL8                0x21a         (538 dec) double : 64 bits, 8 bytes 
#define DATATYPE_REAL16               0x21e         (542 dec) long double : 128 bits, 16 bytes 
#define DATATYPE_CMPLX4               0x31a         (794 dec) complex4 : 64 bits, 8 bytes 
#define DATATYPE_CMPLX8               0x31e         (798 dec) complex8 : 128 bits, 16 bytes 
#define DATATYPE_CMPLX16              0x322         (802 dec) complex16 : 256 bits, 32 bytes 
#define DATATYPE_BITFIELD             0x416         (1046 dec) Bitfield : 32 bits, 4 bytes 
#define DATATYPE_STRING               0x818         (2072 dec) string : 64 bits, 8 bytes 
#define DATATYPE_YYYYMMDD             0x1016        (4118 dec) yyyymmdd : 32 bits, 4 bytes 
#define DATATYPE_HHMMSS               0x2016        (8214 dec) hhmmss : 32 bits, 4 bytes 
#define DATATYPE_LINKOFFSET           0x4017        (16407 dec) linkoffset_t : 32 bits, 4 bytes 
#define DATATYPE_LINKLEN              0x8017        (32791 dec) linklen_t : 32 bits, 4 bytes 
#define DATATYPE_BUFR                 0x10016       (65558 dec) bufr : 32 bits, 4 bytes 
#define DATATYPE_GRIB                 0x20016       (131094 dec) grib : 32 bits, 4 bytes 
#define DATATYPE_BLOB                 0x4004c       (262220 dec) blob64kB : 65536 bytes 
#define DATATYPE_LONGBLOB             0x80088       (524424 dec) blob2GB : 2147483648 bytes 
#define DATATYPE_CHAR                 0x10002c      (1048620 dec) char(1:255) : 256 bytes 
#define DATATYPE_VARCHAR              0x20002c      (2097196 dec) varchar(1:255) : 256 bytes 
*/


char* map_type(int odb_type) {
    switch (odb_type) {
	    case DATATYPE_BITFIELD:
	    case DATATYPE_INT1:
	    case DATATYPE_INT2:
	    case DATATYPE_INT4:
	    case DATATYPE_INT8:
	    case DATATYPE_UINT1:
	    case DATATYPE_UINT2:
	    case DATATYPE_UINT4:
	    case DATATYPE_UINT8:
            case DATATYPE_YYYYMMDD:
            case DATATYPE_HHMMSS:
            case DATATYPE_LINKOFFSET:
	    case DATATYPE_LINKLEN:
	    case DATATYPE_BUFR:
            case DATATYPE_GRIB:
                return "INTEGER";

        case DATATYPE_REAL4:
        case DATATYPE_REAL8:
        case DATATYPE_REAL16:
                return "REAL";

        case DATATYPE_STRING:
	case DATATYPE_CHAR:
        case DATATYPE_VARCHAR:
                return "TEXT";

        default:
                return "REAL";  // fallback 
    }
}

