This is a small collection of ea3x scripts, along with a batch file which
can be used to call a given script on each of the ea3s in the ea3 folder.

e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EA3Scripts>foreach-ea3 extract-length.txt

Executing: extract-length.txt

test1.ea3,61
test2.ea3,126

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EA3Scripts>foreach-ea3-do extract-traj-transl.txt

Executing: extract-traj-transl.txt

test1.ea3,0.000000,0.000000,0.000000
test2.ea3,9.323586,0.000000,-0.086691
