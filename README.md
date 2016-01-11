# banpcars
ProjectCars Simulator Controller

$ ./banpcars.exe -h

BanPCars Server Version : 0.5a

Usage:

banpcars.exe [-r dumpReadFile] [-w dumpWriteFile] [-d startSecs] [-rest port] [-com port]

    -r dumpReadFile  : Read dumpFile instead PCars shared memory
    -w dumpWriteFile : Write dumpFile using active source data.
    -d startSecs     : Only with -r option. Ignore first 'startSecs' of dumpFile
    -rest port       : Start Rest Web Service at 'port' to export data at JSON.
    -com port        : Start Arduino connection at COM 'port'. Only for BanSimBoard.



Example :
    banpcars.exe -r dumpSample.dmp -d 20 -rest 8080 -com 7




Example Rest Usage
	
	GET:
	
		Request: 
			http://localhost:8080/getfields
			
		Response: 
		
			{"fields": [
			   "MVERSION",
			   "MBUILDVERSIONNUMBER",
			   "MGAMESTATE",
			   "MSESSIONSTATE",
			   "MRACESTATE",
			    ...
			   ]
		    }
			
	
	POST 
	
		Request:
			http://localhost:8080/getdata
			
			body :
			
				{"fields": [
				   "MVERSION",
				   "MBUILDVERSIONNUMBER",
				   "MGAMESTATE",
				   "MSESSIONSTATE",
				   "MRACESTATE",
				   "MOILPRESSUREKPA",
					...
				   ]
				}
		
		Response :
		
			{"data": {
			   "MSPLITTIMEAHEAD": -1,
			   "MVERSION": 5,
			   "MGAMESTATE": 2,
			   "MRACESTATE": 1,
			   "MOILPRESSUREKPA": 2.0404673027715932E-14,
			   ...
			   }
		    }
		
		
