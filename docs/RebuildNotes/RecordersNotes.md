
# Recorders
There are two different types of recorders: XML and HDF5 file types



Heirarchical data format (HDF5) standard data file type for standard data.

binary format but platform independent. ex. floating pt numbers arent affected.
self documenting. in file there is a list of data sets, dimensions, types etc.


also integrated with matlab. can load chunks of data. useful for us.

XML just text. advantages with smaller datasets. disadvantage is at sheer volume of data.

set of custom stuff for matlab to allow for

advantages of xml is testing. small sets are easier to read.

legacy: wasnt at point where they wanted to analyze individual spikes.
broke time into bins. 10ms. count of number of spikes in each 10 ms bin. 2014 paper - analysis based on this
todo: bring 2014 paper details into recorders



all neurons probed. all spikes from all neurons are now recorded.
easier to record from everything- didnt slow down. look at subsets.
todo: radii and rates history - change wording to reflect current functionality.




init must be called somewhere else.
this is a wrapper around file. must have been done before initdefault values . blank!


todo: investigate blank initdefaultvalues in recorders class






todo: in param file, put which type of sim so that we can select which thing to record.
todo: why does hdf5 growth recorder redo init? couldnt it just inherit it?

todo: why is hdf5recorder setup to be concrete class? 
todo: structural issues about getting information from connections class.









