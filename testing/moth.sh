#!/bin/bash

# Script name: url_search.sh

# Description: Testing harness for other scripts. Make sure all errors are caught.
#
# Command line options:
# -s   the moth.sh script should immediately exit if any of the tests fail.
# The remaining tests will not be run.
# -h   causes the moth.sh script to produce a useful help message.
#
# Input: the path to the Program Under Test (the PUT) and a text file
# containing a series of argument lists to pass to the PUT
#
# Output: date/time of MOTH run, test number, the command that was executed,
# the received return code and expected return code, whether test passed or failed,
# and overall MOTH test pass or failure.
# Will display only a general help message if -h switch is input.
#
# Special considerations, assumptions, and external references :
# If -h switch is on, MOTH test will not run. Only help message will be displayed.
#

# Help message
h="
Usage:
myprogram is the Program Under Test (the PUT) and myprogram.moth is a text file containing a series of argument lists to pass to the PUT, one list at a time, and the expected return codes for each of those argument lists.
-s means the moth.sh script should immediately exit if any of the tests fail.
-h this helpful message

0   All of the tests passed.
1   There was something wrong with the parameters to your script,
    so an error message and a help should be displayed and the script.
    2   If any of the tests fail and the '-s' option was specified.
    3   If any of the tests fail and the '-s' option was not specified."

    # More than 4 arguments
    if [ "$#" -gt 4 ]; then
	     # echo error, there are more than 4 arguments
	      echo 1>&2 "You entered $# arguments. Too many."
	       # display help
	        echo "$h"
		 exit 1
	 fi

	 # Switches set to default (off)
	 s="n"
	 he="n"

	 # Deal with switches
	 while getopts ":sh" option
	 do
		  case "$option" in
			   s) s="y" ;;
			    h) echo "$h"; he="y"; exit 0 ;;
			     esac
		     done

		     #make sure PUT and test file were specified
		     # OPTIND is built-in variable, counter for switches, always set to 1
		     options=`expr  "$OPTIND" - 1`

		     # Just for testing getopts
		     # echo "options= $options"
		     # echo "OPTIND = $OPTIND::::S# = $#"

		     requiredArgs=`expr "$#" - "$options"`  # subtract away the switches
		     # echo required = $requiredArgs

		     # Nothing here
		     if [ "$requiredArgs" -eq 0  ]
		     then
			        echo 1>&2 "Missing PUT and testfile"
				 # display help
				    echo "$h"
				       exit 1
			       elif  [ "$requiredArgs" -eq 1  ]
			       then
				         # if user only puts in one (non-switch) argument, could be missing either PUT or test file
					   echo 1>&2 "Missing PUT or testfile"
					    # display help
					       echo "$h"
					          exit 1
					  elif   [ "$requiredArgs" -gt 2  ]  # Greater than 2 (non-switch) arguments
					  then
						   # echo error, there are more than 4 total arguments
						      echo 1>&2 "You entered "$requiredArgs" required arguments. Too many."
						       # display help
						          echo "$h"
							     exit 1
						     fi

						     #now we know we have the PUT and testFile, lets shift off options
						     shift $options
						     PUT=$1  # Program under test
						     testFile=$2  # the .moth file
						     # echo myPut=$PUT mytestFile=$testfile    # Testing purposes
						     status="pass"  # So far so good

						     # Extra line
						     echo

						     # if cannot open PUT or read, error out
						     if [ ! -r "$PUT" -o ! -x "$PUT"  ]; then
							       echo "$PUT does not exist or is unexcutable."
							         exit 1
							 fi

							 # if testing file does not end in .moth or doesn't exist, display error + help and exit1
							 if [[ ! "$testFile" =~ .*\.moth$ ]]; then
								  echo "Make sure test file is a .moth file."
								   echo "$h"
								    exit 1
							    fi

							    # If testing file does not exist or is not readable by user
							    if [ ! -r "$testFile" ]; then
								     echo "Make sure $testFile exists or is readable."
								      echo "$h"
								       exit 1
							       fi

							       # echo "MOTH run began at " + date
							       echo "MOTH run began at $(date)"

							       # for every line in $testFile
							       lineNum=1
							       while read line
							       do
								        # parse between last comma
									 # arguments = everything before last comma, and cut quotes
									  # reverse because there could be commas withint quotes, need last comma
									   arguments=$(echo "$line" | rev | cut -d"," -f2-  | rev | sed 's/^.\(.*\).$/\1/' )

									    # expectedReturnCode=everything after last comma
									     expectedReturnCode=$( echo "$line" | rev | cut -d"," -f1  | rev )

									      echo -n "****** running test " $lineNum ": "   # Testing #
									       echo "$PUT" "$arguments"

									        $PUT $arguments > /dev/null 2>&1   # SWALLOW ALL OUTPUT

										 exitCode=$?  # get return code

										  # Does return code match expected return code
										   echo -n return code $exitCode -----
										    if [ $exitCode -eq $expectedReturnCode ]; then
											       echo PASSED
											        else
													   echo FAILED!!! expected return code $expectedReturnCode
													      status=fail  # Set fail marker

													         if [ "$s" = "y" ] ; then  # If -s is turned on
															      exit 2
															         fi      # break immediately

																  fi
																   lineNum=`expr $lineNum + 1`   # increment lineNum
															   done < $testFile

															   #if you can get here, you are good
															   echo "MOTH run ended at $(date)"
															   echo MOTH Result:::: "$status"
															   if [ $status = "pass" ]; then  # Everything's good
																    exit 0
															    elif [ $s = "y" ]; then  # -s switch on
																     exit 2
															     elif [ $s = "n"  -a $he = "n" ]; then  # no s or help turned on
																      exit 3
															      fi
