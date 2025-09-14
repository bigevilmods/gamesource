	package AptOutput;
use strict;
use warnings;
use AptMessage ':all';
use Data::Dumper;
use Carp;
use AptCompiler ':all';
use AptDerive;
use AptDebug;


# Counting vars and logic added by Daniel.
# these vars are exported so that the perl files that create the output files
# can increment them.
our @EXPORT = ( 
our $totalChunks      = 0,
our $totalTris        = 0,
our $totalLines       = 0,
our $totalConstValues = 0,
our $totalTextures = 0,
our $totalTextureSize = 0,
our $totalSounds = 0,
our $totalSoundSize = 0
);

# this flag is used by DefineBitmaps to decide whether or not we want to divide RGB by A
# like we used to do via gx in Apt versions < 2.09.02.
our $unpremultiplyAlpha = 1;

my %bytePositions;
my %jumpPositions;
my @CONSTTABLE;
my %CONSTREF;

# The number of passes we need to get through all the data.  Will be updated as functions
# are called that will require more passes
my $maxPassesNeeded;

# Buffer we may write out to a debug file
my $debugFileBuffer;

my %vftIndex = (
	String => 1,
	Property => 2,
	None => 3,
	Register => 4,
	Boolean => 5,
	Float => 6,
	Integer => 7,
	Lookup => 8,
	Extern => 11
);

sub _uniqueNameForData
{
	my $action = shift;
	my $i = shift;
	my $data = shift;
    if( ! defined($data))
    {
        die "Crap1";
    }
    
    # DC - Added check for SWF::Element::ACTIONDATA::UNDEF. Was causing warnings.
    if ( !$data->isa("SWF::Element::ACTIONDATA::NULL") && !$data->isa("SWF::Element::ACTIONDATA::UNDEF"))
	{
		return "${action}_${i}_$data";
	}
	else
	{
		return "${action}_${i}";
	}
}

# Simple Max function.  I couldn't find one built in to perl for just 2 numbers, all I could find was Max on Lists
# Plus, the list one requires me to pull in another Utility library, which I don't feel like doing for 1 function.
sub max
{
    if ($_[0] > $_[1]) { return $_[0];}
    return $_[1];
}

sub addactionsconstants
{
	my $actions = shift;

	#Carp::cluck("addactionsconstants");

	for (my $i = 0; $i <= $#$actions; $i++)
	{
		my $action = $actions->[$i];
		my $tag = $action->Tag->value;

		if ($tag eq 0x96 || $tag eq 0x88)						# ActionPush || ActionConstantPool (we treat dictionary like push, but it will only ever push strings)
		{
			my @dataList;
			if ($tag eq 0x96)
			{
				@dataList = @{$action->DataList};
			}
			elsif ($tag eq 0x88)
			{
				@dataList = @{$action->ConstantPool};
			}
			else
			{
				die "internal error";
			}

			my $i = 0;
			foreach my $data (@dataList)
			{
    			push @CONSTTABLE, $data;
				my $str = _uniqueNameForData($action, $i, $data);
				$CONSTREF{$str} = $#CONSTTABLE;
				$i++;
			}
		}
		# this code is commented in 0.16.00 version of Apt as
		# the new swfc0.34 does not have WithBlock created for ActionWith.
		# the constants for that block get written as usual constants.
		#elsif ($tag eq 0x94)									# ActionWith
		#{
		#	addactionsconstants($action->WithBlock);
		#}
	}
}

sub addconstantsformovie
{
	my $movie = shift;

	my $nFrames = ($#{$movie} + 1);

	if ($nFrames > 0)					# movie.aFrames
	{
		for (my $i = 0; $i < $nFrames; $i++)
		{
			my $nControls = ($#{$movie->[$i]} + 1);
			if ($nControls > 0)			# apControls
			{
				for (my $j = 0; $j < $nControls; $j++)
				{
					my $control = $movie->[$i][$j];

					if ($control->{type} eq 'PlaceObject2' || $control->{type} eq 'PlaceObject3')
					{
						if (defined $control->{actions})
						{
							foreach my $action (@{$control->{actions}})
							{
								addactionsconstants $action->{actions};
							}
						}
					}
					elsif ($control->{type} eq 'DoAction' || $control->{type} eq 'DoInitAction')
					{
						addactionsconstants $control->{actions};
					}
				}
			}
		}
	}
}

sub writeactionsbody
{
	my $pass = shift;
	my $uniqueString = shift;
	my $actions = shift;
	
    # This function needs at least 3 more passes beyond the first
    $maxPassesNeeded = max $maxPassesNeeded, $pass+3;

	# making this variable (which is used in with action tag) global for this function 
	# was a part of trial and error for getting with block working. 
	# this might be moved back to with block. Apt 0.16.00
	my $withUnique = "" ;
	my $tracenum = 0; # added for new bytecode aptracestart 0x77, Apt 2.03.00
  
  	for (my $i = 0; $i <= $#$actions; $i++)
	{
		my $action = $actions->[$i];
		my $tag = $action->Tag->value;

		if ($curPass == $pass+1)
		{
			if (defined $action->LocalLabel)
			{
				$bytePositions{$uniqueString}{$action->LocalLabel} = getdatasize();
			}
            
            # We are about to write the tag out to the file.  Before that happens,
            # store off a mapping of actions to file offsets so we can later figure 
            # out where this action is in the file
            AptDebug::addActionOffsetMapping($action, getdatasize());
		}
		
		writechar $pass+1, $tag;
		if ($tag eq 0x96 || $tag eq 0x88)						# ActionPush || ActionConstantPool (we treat dictionary like push, but it will only ever push strings)
		{
			align $pass+1, $ptrsize;									# so we can struct hack

			my @dataList;
			if ($tag eq 0x96)
			{
				@dataList = @{$action->DataList};
			}
			elsif ($tag eq 0x88)
			{
				@dataList = @{$action->ConstantPool};
			}
			else
			{
				die "internal error";
			}
			writeint $pass+1, ($#dataList+1);					# nItems
			writeptr $pass+1, "${uniqueString}_${action}_aItems";

			align $pass+2, $ptrsize;
			fixupptr $pass+2, "${uniqueString}_${action}_aItems";
			if ($curPass == $pass+2)
			{
				my $i = 0;
				foreach my $data (@dataList)
				{
				    my $str = _uniqueNameForData($action, $i, $data);
					die "internal error" unless defined $CONSTREF{$str};
					writeintptr $pass+2, $CONSTREF{$str};
					$i++;
				}
			}
				
		}
		elsif ($tag eq 0x9d || $tag eq 0x99 || $tag eq 0xb8)					# ActionIf || ActionJump
		{
			# somewhat better with new version of swf-file.
			# BranchOffset in the action is "label#number". number is the original byte offset, which we ignore.
			# label appears in another instruction as a "LocalLabel" field. So, on pass+1 we write out the jump instruction.
			# on pass+2 we fixup the actual jump target to be our byte offset jump.
			if ($curPass == $pass+1 || $curPass == $pass+2)
			{
				my $branchTo = $action->BranchOffset;
				my $branchLabel = (split /#/, $branchTo)[0];
				if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchLabel})
				{
					#warn "warning: branch to not an instruction; jumping to end of block: $branchTo, branchLabel: $branchLabel";
					# this can actually happen due to a bug in SWF-File. ask Scott about it.
					$branchLabel = "ENDOFBLOCK";
				}
				align $pass+1, $ptrsize;
				writedelayedint $pass+1, "${uniqueString}_${action}";
				if ($curPass == $pass+1)
				{
					$jumpPositions{"${uniqueString}_${action}"} = getdatasize();
				}
				if ($curPass == $pass+2)
				{
					my $delta = $bytePositions{$uniqueString}{$branchLabel} - $jumpPositions{"${uniqueString}_${action}"};
					#print "branch fixup to $delta\n";
					fixupint $pass+2, "${uniqueString}_${action}", $delta;
				}
			}
		}
		elsif ($tag eq 0x81)									# ActionGotoFrame
		{
			align $pass+1, $ptrsize;									# so we can struct hack
			writeint $pass+1, $action->Frame;					# nFrame
		}
		elsif ($tag eq 0x9f)									# ActionGotoFrame2
		{
			align $pass+1, $ptrsize;
			writeint $pass+1, $action->PlayFlag;				# bPlay
		}
		elsif ($tag eq 0x8b)									# ActionSetTarget
		{
			align $pass + 1, $ptrsize;
			writeptr $pass + 1, "${uniqueString}_${action}_targetName";

			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_targetName";
			writestring $pass + 2, $action->TargetName->value;
			#print "settarget:'" . $action->TargetName->value . "'\n";
		}
		elsif ($tag eq 0x8c)									# ActionGotoLabel
		{
			align $pass + 1, $ptrsize;
			writeptr $pass + 1, "${uniqueString}_${action}_targetName";

			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_targetName";
			writestring $pass + 2, $action->Label->value;
			#print "gotolabel:'" . $action->Label->value . "'\n";
		}
		elsif ($tag eq 0x83)									# ActionGetUrl
		{
			#print "GetURL: " . $action->URLString->value . " " . $action->WinString->value . "\n" if $curPass == $pass;
			#print "GetURL: ${$action->URLString} ${$action->WinString}\n" if $curPass == $pass;
			align $pass + 1, $ptrsize;
			writeptr $pass + 1, "${uniqueString}_${action}_URLString";
			writeptr $pass + 1, "${uniqueString}_${action}_WinString";

			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_URLString";
			writestring $pass + 2, $action->UrlString->value;
			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_WinString";
			writestring $pass + 2, $action->TargetString->value;
		}
		elsif ($tag eq 0x94)									# ActionWith
		{
			# new code is added here in for Apt 0.16.00
			# The new swfc0.34 does not have a withBlock in it so the byte code inside 
			# with block is treated same as normal bytecode and extra recursive call to 
			# writeactionsbody is not executed, but becuase of that it is difficult to
			# get the exact code size for with block. 
			# so used the same technique as definefunction where you mark the byte positions in an hash table
			# with unique string and then when in consecutive passes calculate the difference between
			# beginning and end of with block code and write it to .apt file.
			
			align $pass + 1, $ptrsize;
			$withUnique = $uniqueString . "_with_" . $action;
			# commented in 0.16.00
			#writedelayedint $pass+1, "${withUnique}_skip";
			#writeactionsbody($pass, $withUnique, $action->WithBlock);
			#fixupint $pass+1, "${withUnique}_skip", getdatasize();
			
			if ($curPass == $pass+1 || $curPass == $pass+2)
			{
				my $branchTo = $action->CodeSize;
				my $branchLabel = (split /#/, $branchTo)[0];
				if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchLabel})
				{
					print Dumper($bytePositions{$uniqueString});
					die "branch to not an instruction branchTo: $branchTo, branchLabel: $branchLabel";
				}
				#writedelayedint $pass+1, "${uniqueString}_${action}";		# nCodeSize
				writedelayedintptr $pass+1, "${withUnique}_skip";				        # pEnd
				if ($curPass == $pass+1)
				{
					$jumpPositions{"${withUnique}_skip"} = getdatasize();
				}
				if ($curPass == $pass+2)
				{
					my $delta = $bytePositions{$uniqueString}{$branchLabel} - $jumpPositions{"${withUnique}_skip"};
					print "branch fixup to $delta\n";
					fixupintptr $pass+2, "${withUnique}_skip", $delta;
				}
			}
			
			
		}
		elsif ($tag eq 0x00)										# Action End
		{
			# this was a part of trial error for with action tag.
			# this can be removed later as it is not needed.
			#fixupint $pass+1, "${withUnique}_skip", getdatasize();	
		}
		elsif ($tag eq 0x9b)									# ActionDefineFunction
		{
			my @params = @{$action->Params};

			align $pass + 1, $ptrsize;
			writeptr $pass + 1, "${uniqueString}_${action}_FunctionName";	# szName
			writeint $pass + 1, $#params + 1;								# nParams
			writeptr $pass + 1, "${uniqueString}_${action}_aszParams";		# aszParams

			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_FunctionName";
			writestring $pass + 2, $action->FunctionName->value;
			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_aszParams";	
			foreach my $param (@params)
			{
				writeptr $pass + 2, "${uniqueString}_${action}_${param}";
				align $pass + 3, $ptrsize;
				fixupptr $pass + 3, "${uniqueString}_${action}_${param}";
				writestring $pass + 3, $param;								# aszParams[i]
			}

			if ($curPass == $pass+1 || $curPass == $pass+2)
			{
				my $branchTo = $action->CodeSize;
				my $branchLabel = (split /#/, $branchTo)[0];
				if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchLabel})
				{
					print Dumper($bytePositions{$uniqueString});
					die "branch to not an instruction branchTo: $branchTo, branchLabel: $branchLabel";
				}
				writedelayedint $pass+1, "${uniqueString}_${action}";		# nCodeSize
				
				
				align $pass + 1, $ptrsize;										# begin constant pool struct, align to pointer size
				
				writeint	$pass + 1, 0x98765432;								# constantPool.nItems
				writeintptr $pass + 1, 0x12345678;								# constantPool.aItems
				
				if ($curPass == $pass+1)
				{
					$jumpPositions{"${uniqueString}_${action}"} = getdatasize();
				}
				if ($curPass == $pass+2)
				{
					my $delta = $bytePositions{$uniqueString}{$branchLabel} - $jumpPositions{"${uniqueString}_${action}"};
					#print "branch fixup to $delta\n";
					fixupint $pass+2, "${uniqueString}_${action}", $delta;
				}
			}
		}
		elsif ($tag eq 0x87)									# ActionStoreRegister
		{
			align $pass + 1, $ptrsize;
			writeint $pass + 1, $action->Register;
		}
		#-----------------start of newly added opcodes.
		# added in release 0.15.00 First Date 04/02/2004
		#below are newly added opcodes from flasm optimizations.

#_create_action_tag('_BRANCHIFFALSE',0xb8, Value => '$UI8');    # branch if false (jz)
		elsif ($tag eq 0xa1 || $tag eq 0xa4 || $tag eq 0xa5 || $tag eq 0xa6 || $tag eq 0xa7)
		{
			#_create_action_tag('_PUSHS',		0xa1, Value => 'STRING') ;
			#_create_action_tag('_PUSHSGV ',		0xa4, Value => 'STRING');    # push string, get var
			#_create_action_tag('_PUSHSGM ',		0xa5, Value => 'STRING');    # push string, get member
			#_create_action_tag('_PUSHSSV ',		0xa6, Value => 'STRING');    # push string, set var
			#_create_action_tag('_PUSHSSM ',		0xa7, Value => 'STRING');    # push string, set member

			align $pass + 1, $ptrsize;
			writeptr $pass + 1, "${uniqueString}_${action}_Value";

			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_Value";
			writestring $pass + 2, $action->Value->value;
			#print "settarget:'" . $action->TargetName->value . "'\n";
			
		}
		elsif ($tag eq 0xb0 || $tag eq 0xb1 || $tag eq 0xb2 ||$tag eq 0xb3 || $tag eq 0xb5 || $tag eq 0xb9 || $tag eq 0xae || $tag eq 0xaf || $tag eq 0xa2)
		{
			#_create_action_tag('_DCALLFP',		0xb0, Value => '$UI8') ;   # dict call func and pop
			#_create_action_tag('_DCALLFSV',		0xb1, Value => '$UI8') ;   # dict call func and set var
			#_create_action_tag('_DCALLMP',		0xb2, Value => '$UI8') ;   # dict call method and pop
			#_create_action_tag('_DCALLMSV',		0xb3, Value => '$UI8') ;   # dict call method and set var
			#_create_action_tag('_PUSHBYTE',		0xb5, Value => '$UI8');    # push byte
			#_create_action_tag('_PUSHSDBGV',	0xae, Value => '$UI8');    # push string in dict (byte), and get var
			#_create_action_tag('_PUSHSDBGM',	0xaf, Value => '$UI8');    # push string in dict (byte), and get member
			#_create_action_tag('_PUSHSDB',		0xa2, Value => '$UI8');    # push string in dict (byte)
                        #_create_action_tag('_PUSHREGISTER' ,0xb9, undef, Value => '$UI8');    # push register            
			#align $pass + 1, $ptrsize;
			writechar $pass + 1, $action->Value ;			
		}
		elsif ($tag eq 0xb6 || $tag eq 0xa3)
		{
			#_create_action_tag('_PUSHSHORT',	0xb6, Value => '$UI16');    # push word
			#_create_action_tag('_PUSHSDW',		0xa3, Value => '$UI16') ;   # push string in dict (word)			
			#align $pass + 1, $ptrsize;
			writeshort $pass + 1, $action->Value ;
		}
		elsif ($tag eq 0xb7)
		{
			#_create_action_tag('_PUSHLONG',		0xb7, Value => '$UI32');    # push dword
			#align $pass + 1, $ptrsize;
			writeint $pass + 1, $action->Value ;			
		}
		elsif ($tag eq 0xb4)
		{
			#_create_action_tag('_PUSHFLOAT',	0xb4, Value => '$UI32');    # push float
			#align $pass + 1, $ptrsize;
			#actually this should be writefloat but cant find corresponding $UI32 type for float so used $UI32
			writeint $pass + 1, $action->Value ;
			#printf("float - %f \n",$action->Value);			
		}
		elsif ($tag eq 0x77)
		{
			# new bytecode added for Apt 2.03.00
			# this is introduced by atack compiler when it sees AptTrace() before trace statement.
			# we will try to wtite the offset to jump to after 'trace' bytecode.
			if ($curPass == $pass+1 || $curPass == $pass+2)
			{
				my $branchLabel = "AfterTraceOffset${tracenum}";
				#print "AfterTraceOffset ${tracenum} \n";
				if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchLabel})
				{
					warn "warning: branch to not an instruction; jumping to end of block: branchLabel: $branchLabel";
					# this can actually happen due to a bug in SWF-File. 
					$branchLabel = "ENDOFBLOCK";
				}
				# right now it writes an integer for storing offset of trace but we can actually use a short.
				# tried writing short but having some problems with endianness.
				writedelayedint $pass+1, "${uniqueString}_${action}_tracestart_${tracenum}";
				if ($curPass == $pass+1)
				{
					$jumpPositions{"${uniqueString}_${action}_tracestart_${tracenum}"} = getdatasize();
				}
				if ($curPass == $pass+2)
				{
					my $delta = $bytePositions{$uniqueString}{$branchLabel} - $jumpPositions{"${uniqueString}_${action}_tracestart_${tracenum}"};
					$delta += 4; # add 2 bytes for the short.
					print "trace fixup to $delta\n";
					fixupint $pass+2, "${uniqueString}_${action}_tracestart_${tracenum}", $delta;
					#writeshort $pass+2, $delta;  #write actual short now on pass 2
				}
			}
		}
		elsif ($tag eq 0x26)
		{
			my $traceUniqueName = "AfterTraceOffset${tracenum}";
			$bytePositions{$uniqueString}{$traceUniqueName} = getdatasize();
			$tracenum++;            #we increament trace number after we find the actual trace bytecode
		}

		# added this new action tag in Apt 0.17.00.
		# this was possible because of upgrade to swfc 0.34.
		elsif ($tag eq 0x8f)									# ActionTry
		{
		    # uint32_t            uTryCodeSize;               // # of bytes of code that follow (after the Struct)
            # uint32_t            uCatchCodeSize;             // # of bytes of code that follow (After Try Block)
            # uint32_t            uFinallyCodeSize;           // # of bytes of code that follow (After catch Block)
            # uint8_t             uFlags;                     // Flags (See AptDTryCatchBlockFlagsType) Use functions to access.
            # uint8_t             uAlignment1;                // 
            # uint8_t             uAlignment2;                // 
            # uint8_t             uCaughtRegister;            // Register # to put caught object in.
            # char               *szCaughtVarName;            // Var Name to put caught object in.
			
			align $pass + 1, $ptrsize;

            writedelayedint $pass+1, "${uniqueString}_${action}_TrySize";		# uTryCodeSize
            writedelayedint $pass+1, "${uniqueString}_${action}_CatchSize";	    # uCatchCodeSize
            writedelayedint $pass+1, "${uniqueString}_${action}_FinallySize";	# uFinallyCodeSize
			writechar  $pass+1, ${action}->Flags;              # uFlags
			writechar  $pass+1, 0;                             # uAlignment1
			writechar  $pass+1, 0;                             # uAlignment2
			
			if( defined ($action->CatchRegister) )
			{
			    writechar  $pass+1, $action->CatchRegister;	   # uCaughtRegister
			    
			    if ($action->CatchRegister >=32 )
				{
					print "WARNING catch register in try block is above 32\n" ;
				}
			}
			else
			{
			    writechar  $pass+1, 0;	                       # uCaughtRegister
			}
			    
			if( defined ($action->CatchName) )
			{
			    writeptr  $pass+1, "${uniqueString}_${action}_CatchName";  # szCaughtVarName
			    
			    # write String
			    align $pass + 2, $ptrsize;
				fixupptr $pass + 2, "${uniqueString}_${action}_CatchName";
				writestring $pass + 2, $action->CatchName;
			}
			else
			{
			    writeintptr  $pass+1, 0;	                       # szCaughtVarName
			}
			
			if ($curPass == $pass+1 || $curPass == $pass+2)
			{
				my ( $branchTryLabel, $branchCatchLabel, $branchFinallyLabel ) ;
				my $branchTry = $action->TrySize;
				if (defined ($branchTry))
				{
					$branchTryLabel = (split /#/, $branchTry)[0];
				}
				#if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchTryLabel})
				#{
				#	#$branchTryLabel = "ENDOFBLOCK";
				#	#print Dumper($bytePositions{$uniqueString});
				#	#die "branch to not an instruction branchTo: $branchTo, branchLabel: $branchLabel";
				#}
				
				my $branchCatch = $action->CatchSize;
				if (defined ($branchCatch))
				{
					$branchCatchLabel = (split /#/, $branchCatch)[0];
				}
				#if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchCatchLabel})
				#{
				#	#$branchCatchLabel = "ENDOFBLOCK";
				#	#print Dumper($bytePositions{$uniqueString});
				#	#die "branch to not an instruction branchTo: $branchTo, branchLabel: $branchLabel";
				#}

				my $branchFinally = $action->FinallySize;
				if (defined ($branchFinally))
				{
					$branchFinallyLabel = (split /#/, $branchFinally)[0];
				}
				#if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchFinallyLabel})
				#{
				#	#$branchFinallyLabel = "ENDOFBLOCK";
				#	#print Dumper($bytePositions{$uniqueString});
				#	#die "branch to not an instruction branchTo: $branchTo, branchLabel: $branchLabel";
				#}
				
				if ($curPass == $pass+1)
				{
					$jumpPositions{"${uniqueString}_${action}_try"} = getdatasize();
				}
				if ($curPass == $pass+2)
				{
					my $delta = $bytePositions{$uniqueString}{$branchTryLabel} - $jumpPositions{"${uniqueString}_${action}_try"};
					#print "branch fixup to $delta\n";
					fixupint $pass+2, "${uniqueString}_${action}_TrySize", $delta;
					
					if (not defined ($action->CatchSize) || not defined ($action->TrySize))
					{
						$delta = 0 ;						
					}
					else
					{
						$delta = $bytePositions{$uniqueString}{$branchCatchLabel} - $bytePositions{$uniqueString}{$branchTryLabel};
					}
					fixupint $pass+2, "${uniqueString}_${action}_CatchSize", $delta;
					if (not defined ($action->FinallySize)) 
					{
						$delta = 0 ;
					}
					elsif (not defined ($action->CatchSize))
					{
						$delta = $bytePositions{$uniqueString}{$branchFinallyLabel} - $bytePositions{$uniqueString}{$branchTryLabel};
					}
					else
					{
						$delta = $bytePositions{$uniqueString}{$branchFinallyLabel} - $bytePositions{$uniqueString}{$branchCatchLabel};
					}
					fixupint $pass+2, "${uniqueString}_${action}_FinallySize", $delta;
				}
			}
		}
		
		# added this new action tag in Apt 0.16.00.
		# this was possible because of upgrade to swfc 0.34.
		# most of the code is same as definefunction with some extra code.
		elsif ($tag eq 0x8e)									# ActionDefineFunction2
		{
			my @params = @{$action->Parameters};

			align $pass + 1, $ptrsize;
			writeptr $pass + 1, "${uniqueString}_${action}_FunctionName";	# szName
			writeint $pass + 1, $#params + 1;								# nParams
			writeshort $pass + 1, $action->RegisterCount ;					# nRegisterCount			
			writeshort $pass + 1, $action->Flags ;							# nFlags
			writeptr $pass + 1, "${uniqueString}_${action}_aszParams";		# aszParams

			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_FunctionName";
			writestring $pass + 2, $action->FunctionName->value;
			align $pass + 2, $ptrsize;
			fixupptr $pass + 2, "${uniqueString}_${action}_aszParams";	
			# parameters have extra register number in it if register is used
			foreach my $param (@params)
			{
				writeint $pass + 2, $param->Register ;								#nRegister number
				writeptr $pass + 2, "${uniqueString}_${action}_${param}_szParamName";
				align $pass + 3, $ptrsize;
				fixupptr $pass + 3, "${uniqueString}_${action}_${param}_szParamName";
				writestring $pass + 3, $param->ParamName;					# aszParams[i]
			}

			if ($curPass == $pass+1 || $curPass == $pass+2)
			{
				my $branchTo = $action->CodeSize;
				my $branchLabel = (split /#/, $branchTo)[0];
				if ($curPass == $pass+2 && not defined $bytePositions{$uniqueString}{$branchLabel})
				{
					$branchLabel = "ENDOFBLOCK";
					#print Dumper($bytePositions{$uniqueString});
					#die "branch to not an instruction branchTo: $branchTo, branchLabel: $branchLabel";
				}
				writedelayedint $pass+1, "${uniqueString}_${action}";		# nCodeSize
				
				align $pass + 1, $ptrsize;										# begin constant pool struct, align to pointer size
				
				writeint    $pass + 1, 0x98765432;								# constantPool.nItems
				writeintptr $pass + 1, 0x12345678;								# constantPool.aItems
				if ($curPass == $pass+1)
				{
					$jumpPositions{"${uniqueString}_${action}"} = getdatasize();
				}
				if ($curPass == $pass+2)
				{
					my $delta = $bytePositions{$uniqueString}{$branchLabel} - $jumpPositions{"${uniqueString}_${action}"};
					#print "branch fixup to $delta\n";
					fixupint $pass+2, "${uniqueString}_${action}", $delta;
				}
			}
		}
		
		#----------end of newly added opcodes
	}
	if ($curPass == $pass+1)
	{
		$bytePositions{$uniqueString}{"ENDOFBLOCK"} = getdatasize();
	}
}

sub writeactions($$$)
{
	my $pass = shift;
	my $uniqueString = shift;
	my $actions = shift;
	
    # This function needs at least 1 more pass beyond the first
    $maxPassesNeeded = max $maxPassesNeeded, $pass+1;

	Carp::cluck "writeactions doesn't want 0 actions" if ($#$actions == -1);

	lengthcheckstart("actions");

	writeptr $pass, "${uniqueString}_aActionStream";

=pod
	if ($curPass == $pass)
	{
		foreach my $action (@$actions)
		{
			print $action->ByteOffset->value . ": " . $action->Tag->value . "\n";
		}
	}
=cut

	align $pass+1, $ptrsize;
	fixupptr $pass+1, "${uniqueString}_aActionStream";
	writeactionsbody $pass, $uniqueString, $actions;

	lengthcheckstop("actions");
}

sub writemoviebase
{
	my $pass = shift;
	my $movie = shift;

	lengthcheckstart("moviebase");

	my $nFrames = ($#{$movie} + 1);
	writeint $pass, $nFrames;				# movie.nFrames
	
	align $pass, $ptrsize;
	
	if ($nFrames > 0)						# movie.aFrames
	{ 
		writeptr $pass, "${movie}_aFrames"; 
	}
	else 
	{ 
		writeintptr $pass, 0; 
	}				
	
	writeintptr $pass, 0;						# movie.phLabels

	lengthcheckstop("moviebase");
}

sub writefilter
{
    my ($pass, $filt, $ptrname) = @_;
    my $filter_id = $filt->FilterID;
        
    align $filter_id, 4;
    
    writeint $pass, $filter_id;
    
    if ( $filter_id eq 0 ) {
        # DropShadow
        writecolour $pass, $filt->DropShadow->DropShadowColor;
        writeint $pass, $filt->DropShadow->BlurX;
        writeint $pass, $filt->DropShadow->BlurY;
        writeint $pass, $filt->DropShadow->Angle;
        writeint $pass, $filt->DropShadow->Distance;
        writeshort $pass, $filt->DropShadow->Strength;
        writeshort $pass, $filt->DropShadow->Flags;
    }
    elsif ( $filter_id eq 1 ) {
        # Blur
        writeint $pass, $filt->Blur->BlurX;
        writeint $pass, $filt->Blur->BlurY;
        writeshort $pass, $filt->Blur->Flags; 	    
    }
    elsif ( $filter_id eq 2 ) {
        # Glow
        writecolour $pass, $filt->Glow->GlowColor;
        writeint $pass, $filt->Glow->BlurX;
        writeint $pass, $filt->Glow->BlurY;
        writeshort $pass, $filt->Glow->Strength;
        writeshort $pass, $filt->Glow->Flags;                
    }
    elsif ( $filter_id eq 3 ) {
        # Bevel
        writecolour $pass, $filt->Bevel->HighlightColor;
        writecolour $pass, $filt->Bevel->ShadowColor;
        writeint $pass, $filt->Bevel->BlurX;
        writeint $pass, $filt->Bevel->BlurY;
        writeint $pass, $filt->Bevel->Angle;
        writeint $pass, $filt->Bevel->Distance;        
        writeshort $pass, $filt->Bevel->Strength;
        writeshort $pass, $filt->Bevel->Flags;                
    }   
    elsif ( $filter_id eq 4 || $filter_id eq 7 ) {
        # GradientGlow and GradientBevel
        my $grad_filter = $filt->GradientGlow;
        if ( $filter_id eq 7 )
        {
            $grad_filter = $filt->GradientBevel;
        }
        align $pass, 4;
        writeint $pass, $grad_filter->NumColors;
                                
        writeptr $pass, "${ptrname}_GradColors";

        align $pass+1, 4;
        fixupptr $pass+1, "${ptrname}_GradColors";

        my $color_list = $grad_filter->GradientColors;
        foreach my $gradcolor (@$color_list)
        {
            writecolour $pass+1, $gradcolor;
        }

        writeptr $pass, "${ptrname}_GradRatios";

        align $pass+1, 4;
        fixupptr $pass+1, "${ptrname}_GradRatios";

        my $ratio_list = $grad_filter->GradientRatios;
        foreach my $gradratio (@$ratio_list)
        {
            writechar $pass+1, $gradratio->RatioValue;
        }
        align $pass,   4;
        align $pass+1, 4;
        
        writeint $pass, $grad_filter->BlurX;
        writeint $pass, $grad_filter->BlurY;
        writeint $pass, $grad_filter->Angle;
        writeint $pass, $grad_filter->Distance;
        writeshort $pass, $grad_filter->Strength;
        writeshort $pass, $grad_filter->Flags;
    }   
    elsif ( $filter_id eq 5 )
    {
        # Convolution
        # This filter cannot be set in the Flash 8 or Flash CS3 authoring environment.
        # But, we have this code anyway to handle it anyway.
        align $pass, 4;
        writechar $pass, $filt->Convolution->MatrixX;
        writechar $pass, $filt->Convolution->MatrixY;
        writeshort $pass, $filt->Convolution->Flags;
        writeint $pass, $filt->Convolution->Divisor;
        writeint $pass, $filt->Convolution->Bias;
        writeptr $pass, "${ptrname}_MatrixValues";
        align $pass+1, 4;
        fixupptr $pass+1, "${ptrname}_MatrixValues";
        my $mat_values = $filt->Convolution->MatrixValues;
        foreach my $mat_value (@$mat_values)
        {
            writeint $pass+1, $mat_value->Value;
        }
        align $pass,   4;
        align $pass+1, 4;
        writecolour $pass, $filt->Convolution->DefaultColor;        
    }
    elsif ( $filter_id eq 6 ) {
        # ColorMatrix

        writeint $pass, $filt->ColorMatrix->r0; 
        writeint $pass, $filt->ColorMatrix->r1;
        writeint $pass, $filt->ColorMatrix->r2;
        writeint $pass, $filt->ColorMatrix->r3;
        writeint $pass, $filt->ColorMatrix->r4;
        
        writeint $pass, $filt->ColorMatrix->g0;
        writeint $pass, $filt->ColorMatrix->g1;
        writeint $pass, $filt->ColorMatrix->g2;
        writeint $pass, $filt->ColorMatrix->g3; 
        writeint $pass, $filt->ColorMatrix->g4;
        
        writeint $pass, $filt->ColorMatrix->b0;
        writeint $pass, $filt->ColorMatrix->b1;
        writeint $pass, $filt->ColorMatrix->b2;
        writeint $pass, $filt->ColorMatrix->b3;
        writeint $pass, $filt->ColorMatrix->b4;
        
        writeint $pass, $filt->ColorMatrix->a0;
        writeint $pass, $filt->ColorMatrix->a1;
        writeint $pass, $filt->ColorMatrix->a2;
        writeint $pass, $filt->ColorMatrix->a3;
        writeint $pass, $filt->ColorMatrix->a4;
    }
    else {
        die "Unexpected filter type: $filter_id\n";
    }
}

sub writemovieframes
{
	my $pass = shift;
	my $movie = shift;
	my $nFrames = ($#{$movie} + 1);
	
    # This function needs at least 4 more passes beyond the first
    $maxPassesNeeded = max $maxPassesNeeded, $pass+4;

	if ($nFrames > 0)					# movie.aFrames
	{
		align $pass, $ptrsize;
		fixupptr $pass, "${movie}_aFrames";
		
		for (my $i = 0; $i < $nFrames; $i++)
		{
			lengthcheckstart("movieframe ($i)");

			align $pass, $ptrsize;			#begin the AptFrame struct, must align to pointer size
			
			my $nControls = ($#{$movie->[$i]} + 1);
			writeint $pass, $nControls;		# nControls

			if ($nControls > 0)			# apControls
			{
				writeptr $pass, "${movie}_${i}_apControls";

				align $pass+1, $ptrsize;
				
				fixupptr $pass+1, "${movie}_${i}_apControls";
				for (my $j = 0; $j < $nControls; $j++)
				{
					my $control = $movie->[$i][$j];
					lengthcheckstart("$j ($control->{type})");
					writeptr $pass+1, "${movie}_${i}_aControls_${j}";

					align $pass+2, $ptrsize;  #begin the union of structures, need to align to pointer size
					
                    # We are about to write the control tag out to the file.  Before that happens,
                    # store off a mapping of controls to file offsets so we can later figure 
                    # out where this control is in the file
                    if ($curPass == $pass+2)
                    {
                        AptDebug::addControlOffsetMapping($control, getdatasize());
                    }
					
					if ($control->{type} eq 'PlaceObject2' || $control->{type} eq 'PlaceObject3')
					{
						align $pass+2, $ptrsize;
						fixupptr $pass+2, "${movie}_${i}_aControls_${j}";
						
						my $control_type_num = 3;                   # AptControlType_PlaceObject2
						if ($control->{type} eq 'PlaceObject3') {						
						    $control_type_num = 9;                  # AptControlType_PlaceObject3
						}
						writeint $pass+2, $control_type_num;
						
						align $pass+2, $ptrsize;
						writeint $pass+2, $control->{flags};			# eFlags
						writeint $pass+2, $control->{depth};			# nDepth
						if (defined $control->{characterid})			# nCharacterID
						{
							writeint $pass+2, $control->{characterid};
						}
						else
						{
							writeint $pass+2, -1;
						}
						lengthcheckstart("transform");
						writematrix $pass+2, $control->{matrix};		# matrix
						writencxform $pass+2, $control->{cxform};		# cxform
						lengthcheckstop("transform");
						if (defined $control->{ratio})					# fRatio
						{
							writefloat $pass+2, $control->{ratio} / 65536;
						}
						else
						{
							writefloat $pass+2, 0;
						}

						if (defined $control->{name})			# szName
						{
							writeptr $pass+2, "${movie}_${i}_apControls_${j}_szName";
							lengthcheckstart("name");
							align $pass+3, 4;
							fixupptr $pass+3, "${movie}_${i}_apControls_${j}_szName";
							writestring $pass+3, $control->{name};
							lengthcheckstop("name");
						}
						else
						{
							writeintptr $pass+2, 0;
						}

						if (defined $control->{clipdepth})		# nClipDepth
						{
							writeint $pass+2, $control->{clipdepth};
						}
						else
						{
							writeint $pass+2, -1;
						}

						if (defined $control->{actions})
						{
							writeptr $pass+2, "${movie}_${i}_apControls_${j}_pActions";		# pActions

							lengthcheckstart("POactions");
							align $pass+3, 4;
							fixupptr $pass+3, "${movie}_${i}_apControls_${j}_pActions";
							writeint $pass+3, ($#{$control->{actions}} + 1);				# nEventActions
							writeptr $pass+3, "${movie}_${i}_apControls_${j}_aEventActions";

							align $pass+4, 4;
							fixupptr $pass+4, "${movie}_${i}_apControls_${j}_aEventActions";

							foreach my $action (@{$control->{actions}})
							{
								writeint $pass+4, $action->{triggerflags};
								writeint $pass+4, $action->{keycode};
								writeactions $pass+4, "${movie}_${i}_apControls_${j}_${action}", $action->{actions};
							}
							lengthcheckstop("POactions");
						}
						else
						{
							writeintptr $pass+2, 0;											# pActions
						}

                        if ( $control->{type} eq 'PlaceObject3' )
                        {
                            if (defined $control->{blendmode})
                            {
                                writeint $pass+2, $control->{blendmode};
                            }
                            else
                            {
                                writeint $pass+2, -1;
                            }

                            if (defined $control->{filters})						    
                            {
                                lengthcheckstart("POfilters");
                                align $pass+2, 4;						        
                                
                                writeint $pass+2, ($#{$control->{filters}} + 1);  # number of filters
                                writeptr $pass+2, "${movie}_${i}_apControls_${j}_aFilters";

                                align $pass+3, 4;
                                fixupptr $pass+3, "${movie}_${i}_apControls_${j}_aFilters";

                                foreach my $filt   (@{$control->{filters}})
                                {                                    
                                    writeptr $pass+3, "${movie}_${i}_apControls_${j}_aFilters_${filt}";
                                    
                                    align $pass+4, 4;
                                    fixupptr $pass+4, "${movie}_${i}_apControls_${j}_aFilters_${filt}";
                                    
                                    writefilter $pass+4, $filt, "${movie}_${i}_apControls_${j}_aFilters_${filt}";
                                }
                                lengthcheckstop("POfilters");
                            }
                            else
                            {
                                writeint $pass+2, 0;
                                writeintptr $pass+2, 0;
                            }                            
                        } # if $control->{type} 
                    } # for my $j
					elsif ($control->{type} eq 'SetBackgroundColor')
					{
						align $pass+2, $ptrsize;
						fixupptr $pass+2, "${movie}_${i}_aControls_${j}";
						
						
						writeint $pass+2, 5;							# eType == AptControlType_BackgroundColour
						
						align $pass+2, $ptrsize;
						writecolour $pass+2, $control->{colour};		# nColour
					}
					elsif ($control->{type} eq 'DoAction')
					{
						align $pass+2, $ptrsize;
						fixupptr $pass+2, "${movie}_${i}_aControls_${j}";
						writeint $pass+2, 1;							# eType == AptControlType_DoAction
						align $pass+2, $ptrsize;
						writeactions $pass+2, "${movie}_${i}_aControls_${j}_actions", $control->{actions};
					}
					elsif ($control->{type} eq 'DoInitAction')
					{
						align $pass+2, $ptrsize;
						fixupptr $pass+2, "${movie}_${i}_aControls_${j}";
						writeint $pass+2, 8;							# eType == AptControlType_DoInitAction
						align $pass+2, $ptrsize;
						writeint $pass+2, $control->{spriteid};
						writeactions $pass+2, "${movie}_${i}_aControls_${j}_actions", $control->{actions};
					}
					elsif ($control->{type} eq 'RemoveObject2')
					{
						align $pass+2, $ptrsize;
						fixupptr $pass+2, "${movie}_${i}_aControls_${j}";
						writeint $pass+2, 4;							# eType == AptControlType_RemoveObject2
						align $pass+2, $ptrsize;
						writeint $pass+2, $control->{depth};			# nDepth
					}
					elsif ($control->{type} eq 'FrameLabel')
					{
						align $pass + 2, $ptrsize;
						fixupptr $pass + 2, "${movie}_${i}_aControls_${j}";
						writeint $pass + 2, 2;							# eType == AptControlType_FrameLabel
						align $pass+2, $ptrsize;
						writeptr $pass + 2, "${movie}_${i}_aControls_${j}_label";
						writeshort $pass + 2, 0;						# aValue.nRefCount
						writeshort $pass + 2, $vftIndex{Integer};		# aValue.__vtblIndex
						writeint $pass + 2, $i;							# aValue.nInteger

						align $pass + 3, 4;
						fixupptr $pass + 3, "${movie}_${i}_aControls_${j}_label";
						writestring $pass + 3, $control->{label};
					}
					elsif ($control->{type} eq 'StartSound')
					{
						align $pass + 2, $ptrsize;
						fixupptr $pass + 2, "${movie}_${i}_aControls_${j}";
						writeint $pass + 2, 6;							# eType == AptControlType_StartSound
						align $pass+2, $ptrsize;
						writeint $pass + 2, $control->{id};
					}
					elsif ($control->{type} eq 'SoundStreamHead')
					{
						align $pass + 2, $ptrsize;
						fixupptr $pass + 2, "${movie}_${i}_aControls_${j}";
						writeint $pass + 2, 7;							# eType == AptControlType_StartSoundStream
						align $pass+2, $ptrsize;
						writeint $pass + 2, $control->{id};
					}
					else
					{
						message 0, "Ignored type on writemovie: $control->{type}\n" if $pass == $curPass;
					}
					lengthcheckstop("$j ($control->{type})");
				}
			}
			else
			{
				writeintptr $pass, 0;    # apControls
			}
			lengthcheckstop("movieframe ($i)");
		}
	}
}

sub _multMatrix($$)
{
	my @m0 = @{$_[0]};
	my @m1 = @{$_[1]};
	my @out;

	for (my $j=0; $j<4; $j++)
	{
		for (my $i=0; $i<4; $i++)
		{
			$out[$j*4 + $i] = $m0[$j*4 + 0]*$m1[0*4 + $i] + 
						      $m0[$j*4 + 1]*$m1[1*4 + $i] +
							  $m0[$j*4 + 2]*$m1[2*4 + $i] +
							  $m0[$j*4 + 3]*$m1[3*4 + $i];
		}
	}
	return @out;
}

sub _writeShape($$$)
{
	my $fileChunks = 0;
	my $fileTris   = 0;
	my $fileLines  = 0;
	my $textID = -1;
			
	my ($character, $i ,$forceempty) = @_;

	my $fileName = "$main::outdir{geom}\\${i}.ru";
	
#	print Dumper(\@_);

	open RU, ">$fileName" or die ">$fileName";

	# nasty hack to workaround confusion in geometry compilers: output one hidden triangle if nothing there
	# even more nasty now, because of the forceempty, see where _writeShape get called for more information
	if ((scalar(@{$character->{shapes}}) == 0)  ||  ($forceempty == 1))
	{
		displayInfo "geometry", "$fileName : Chunks:1 Triangles:1 Lines:1\n";
		$totalChunks = $totalChunks + 1;
		$totalTris   = $totalTris   + 1;
		$totalLines  = $totalLines  + 1;
		print RU "c\n";
		print RU "s s:0:0:0:0\n";
		print RU "t 0:0:0:0:0:0\n";
		close RU;
		return $textID;
	}

	foreach my $independentShape (@{$character->{shapes}})
	{
		my %tridata;

		$fileChunks = $fileChunks + 1;
		
		print RU "c\n";
		my $numTris = ($#{$independentShape->{triangles}} + 1);
		if ($numTris > 0)
		{
			foreach my $tri (@{$independentShape->{triangles}})
			{
				my $style = $tri->{style};
				my $tritext = 
					$tri->{a}[0] / 20.0 . ":" .
					$tri->{a}[1] / 20.0 . ":" .
					$tri->{b}[0] / 20.0 . ":" .
					$tri->{b}[1] / 20.0 . ":" .
					$tri->{c}[0] / 20.0 . ":" .
					$tri->{c}[1] / 20.0;

				if ($style->FillStyleType == 0)	# solid fill
				{
					push @{$tridata{"s:" . colourColoned($tri->{style}->Color)}}, $tritext;
				}
				elsif ($style->FillStyleType == 0x10 || $style->FillStyleType == 0x12 || $style->FillStyleType == 0x13) # linear/radial gradient fill
				{
					my $invmtx = invertmatrix(
							[$style->BitmapMatrix->ScaleX / 20, $style->BitmapMatrix->RotateSkew0 / 20,
							$style->BitmapMatrix->RotateSkew1 / 20, $style->BitmapMatrix->ScaleY / 20,
							$style->BitmapMatrix->TranslateX / 20, $style->BitmapMatrix->TranslateY / 20]);

					my @afMatrix = (
						$invmtx->[0], $invmtx->[1], 0, 0,
						$invmtx->[2], $invmtx->[3], 0, 0,
						0, 0, 1, 0,
						$invmtx->[4], $invmtx->[5], 0, 1 );
					my @gradMatrix;

					if ($style->FillStyleType == 0x10)
					{
						@gradMatrix = (
							256 / 32768, 0, 0, 0,
							0, 1   / 32768, 0, 0,
							0, 0, 1, 0,
							0.5*256, -0.5, 0, 1);
					}
					else
					{
			            # 0x12 - radial gradient
			            # 0x13 - focal radial gradient (unsupported; treated same as radial gradient)					
						@gradMatrix = (
							$::GradientCircleSize / 32768, 0, 0, 0,
							0, $::GradientCircleSize / 32768, 0, 0,
							0, 0, 1, 0,
							0.5*$::GradientCircleSize, 0.5*$::GradientCircleSize, 0, 1);
					}
					my @mat = _multMatrix(\@afMatrix, \@gradMatrix);

					$textID = $style->BitmapID;
					
					push @{$tridata{sprintf("%s:%s:%d:%f:%f:%f:%f:%f:%f",
							"tc",
							colourColoned($style->Color),
							$style->BitmapID,
							$mat[0], $mat[1], $mat[4], $mat[5], $mat[12], $mat[13])}},
					$tritext;
				}
				elsif ($style->FillStyleType == 0x40				# tiled bitmap fill
						|| $style->FillStyleType == 0x41			# clipped bitmap fill
						|| $style->FillStyleType == 0x42			# ??? bitmap fill
						|| $style->FillStyleType == 0x43)			# ??? bitmap fill
				{
					my $invmtx = invertmatrix(
							[$style->BitmapMatrix->ScaleX / 20, $style->BitmapMatrix->RotateSkew0 / 20,
							$style->BitmapMatrix->RotateSkew1 / 20, $style->BitmapMatrix->ScaleY / 20,
							$style->BitmapMatrix->TranslateX / 20, $style->BitmapMatrix->TranslateY / 20]);

					$textID = $style->BitmapID;

					push @{$tridata{sprintf("%s:%s:%d:%f:%f:%f:%f:%f:%f",
							$style->FillStyleType == 0x40 ? "tw" : "tc",
							colourColoned($style->Color),
							$style->BitmapID,
							$invmtx->[0], $invmtx->[1], $invmtx->[2], $invmtx->[3], $invmtx->[4], $invmtx->[5])}},
					$tritext;
				}
				else
				{
					die "internal error: unhandled style";
				}
			}
			
			while (my ($key, $data) = each(%tridata))
			{
				print RU "s $key\n";
				foreach my $triangle (@$data)
				{
					$fileTris = $fileTris + 1;
					print RU "t $triangle\n";
				}
			}
		}
		my $numLines = ($#{$independentShape->{lines}} + 1);
		if ($numLines > 0)
		{
			my %linedata;
			foreach my $line (@{$independentShape->{lines}})
			{
				push @{$linedata{"l:" . $line->{style}->Width / 20.0 . ":" . colourColoned($line->{style}->Color)}},
				$line->{from}[0] / 20.0 . ":" .
					$line->{from}[1] / 20.0 . ":" .
					$line->{to}[0] / 20.0 . ":" .
					$line->{to}[1] / 20.0;
			}

			while (my ($key, $data) = each(%linedata))
			{
				print RU "s $key\n";
				foreach my $line (@$data)
				{
					$fileLines = $fileLines + 1;
					print RU "l $line\n";
				}
			}
		}
		if ($numLines == 0 && $numTris == 0)
		{
			print RU "c\n";
			$fileTris = $fileTris + 1;
			$fileLines = $fileLines + 1;
			print RU "s s:0:0:0:0\n";
			print RU "t 0:0:0:0:0:0\n";
			close RU;
			return $textID;
		}
	}

	displayInfo "geometry", "$fileName : Chunks:$fileChunks Triangles:$fileTris Lines:$fileLines\n";
	
	$totalChunks = $totalChunks + $fileChunks;
	$totalTris   = $totalTris   + $fileTris;
	$totalLines  = $totalLines  + $fileLines;
	
	close RU;
	return $textID;
}

sub _calcIndex($$$$)
{
	my $vertlist = $_[0];
	my $indexlist = $_[1];
	my $x = $_[2];
	my $y = $_[3];
	my $vertlength = scalar(@$vertlist);
	my $theIndex = -1;
	for(my $i=0;$i<$vertlength;$i+=2)
	{
		if(	($vertlist->[$i] == $x) &&
			($vertlist->[$i+1] == $y)	)
		{
			$theIndex = $i /2;
			last;
		}
	}
	#didn't find 
	if($theIndex == -1)
	{
		push(@$vertlist,$x);
		push(@$vertlist,$y);
		$theIndex = $vertlength/2;
	}
	push (@$indexlist,$theIndex);
}

sub _getHitTestTriangles
{
	my $charID = shift;
	my $characters = shift;
	my $character = $characters->[$charID];
	if ($character->{type} ne 'button')
	{
		die "_getHitTestTriangles should be only invoked on button charactors\n";
	}

	my @vertlist;
	my @indexlist;
	foreach my $br (@{$character->{buttonrecords}})
	{
		if(($br->{states} & 8) != 0)	# 8 in states means it's the hit test shape
		{
			my $hittestcharacter = $characters->[$br->{character}];
#			if the test region is a shape
			if($hittestcharacter->{type} eq 'shape')
			{
				foreach my $independentShape (@{$hittestcharacter->{shapes}})
				{		
					foreach my $tri (@{$independentShape->{triangles}})
					{
						_calcIndex(\@vertlist,\@indexlist,$tri->{a}[0] / 20.0,$tri->{a}[1] / 20.0);
						_calcIndex(\@vertlist,\@indexlist,$tri->{b}[0] / 20.0,$tri->{b}[1] / 20.0);
						_calcIndex(\@vertlist,\@indexlist,$tri->{c}[0] / 20.0,$tri->{c}[1] / 20.0);
					}
				}
				return (\@vertlist,\@indexlist);
			}
		}
	}
#	if the code reaches here, possibly because the test region is not a shape (eg. a text or something else), in this case, we return an empty array
	return (\@vertlist,\@indexlist);
}


sub _findTestBoundingRect
{
	my $charID = shift;
	my $characters = shift;
	my $character = $characters->[$charID];
	if ($character->{type} ne 'button')
	{
		die "_findTestBoundingRect should be only invoked on button charactors\n";
	}

	foreach my $br (@{$character->{buttonrecords}})
	{
		if(($br->{states} & 8) != 0)	# 8 in states means it's the hit test shape
		{
			my $hittestcharacter = $characters->[$br->{character}];
#			actually, it could be not a shape, at least I saw it can be a text which is handled by apt now
#			so, I comment out the following line, and in case it's not a shape, we return in invalid region			
#			die "hittest character is not a shape?!\n" if($hittestcharacter->{type} ne 'shape');

			die "no bounding rect for hit test shape!?!\n" unless (defined($hittestcharacter->{boundingRect}));
			# print "bounding box :",$hittestcharacter->{boundingRect}[0] / 20.0,":",$hittestcharacter->{boundingRect}[1] / 20.0,":",$hittestcharacter->{boundingRect}[2] / 20.0,":",$hittestcharacter->{boundingRect}[3] / 20.0,"\n";
			return ($hittestcharacter->{boundingRect}[0] / 20.0,$hittestcharacter->{boundingRect}[1] / 20.0,$hittestcharacter->{boundingRect}[2] / 20.0,$hittestcharacter->{boundingRect}[3] / 20.0);
		}
	}
#	return an invalid region when the hittest region is not a shape	
	return (0,0,-1,-1);
}


sub writeConstTable
{
	my %constFileStrings;
	my $constFileName = shift;
	# if $constFilename is defined then output all the values
	if ($constFileName)
	{
		open CONST, ">$constFileName" or die "couldn't open '$constFileName' for output";
		binmode(CONST, ":crlf");
	}
    # (LEO) added this for property support..
    my %propNum = 
	( _x            => 0,
	  _y            => 1,
      _xscale       => 2,
      _yscale       => 3,
	  _currentframe => 4,
	  _totalframes  => 5,
	  _alpha        => 6,
	  _visible      => 7,
	  _width        => 8,
	  _height       => 9,
	  _rotation     => 10,
	  _target       => 11,
	  _framesloaded => 12,
	  _name         => 13,
	  _droptarget   => 14,
	  _url          => 15,
	  _highquality  => 16,
	  _focusrect    => 17,
	  _soundbuftime => 18,
	  _quality      => 19,
	  _xmouse       => 20,
	  _ymouse       => 21,
	);    
	
	for ($curPass = 1; $curPass <= 3; $curPass++)
	{
		writeint 1, scalar(@CONSTTABLE);
		writeptr 1, "aConstants";
		align 2, 4;
		fixupptr 2, "aConstants";
				
		my $index = 0;
		
		$totalConstValues = 0;
		
		foreach my $data (@CONSTTABLE)
		{
			$totalConstValues = $totalConstValues + 1;
			
			align 2, $ptrsize;
			
			if ($data->isa('SWF::Element::ACTIONDATA::String') || $data->isa('SWF::Element::STRING'))
			{
				writeint 2, $vftIndex{String};
				writeptr 2, "${index}_{data}";
				align 3, 4;
				fixupptr 3, "${index}_{data}";
				writestring 3, $data->value;		# szString
				if ($constFileName)
				{
					$constFileStrings{$data->value} = "String";# = ($data->value => " :String\n");
				}
				
			}
			elsif ($data->isa('SWF::Element::ACTIONDATA::Double'))
			{
				writeint 2, $vftIndex{Float};
				align	 2, $ptrsize;
				writefloat 2, $data->value;			# fFloat
			}
			elsif ($data->isa('SWF::Element::ACTIONDATA::Integer'))
			{
				writeint 2, $vftIndex{Integer};
				align	 2, $ptrsize;
				writeint 2, $data->value;			# nInteger
			}
			elsif ($data->isa('SWF::Element::ACTIONDATA::Lookup') || $data->isa('SWF::Element::ACTIONDATA::LookupLong'))
			{
				writeint 2, $vftIndex{Lookup};
				align	 2, $ptrsize;
				writeint 2, $data->value;			# nLookup
			}
			elsif ($data->isa('SWF::Element::ACTIONDATA::Property'))
			{
				# just push them as floats because flash sometimes does anyway so we already handle that
				writeint 2, $vftIndex{Float};
				align	 2, $ptrsize;
				writefloat 2, $propNum{$data->value};			# fFloat    (LEO) updated for property support
				if ($constFileName)
				{
					$constFileStrings{$data->value} = "Property";# = ($data->value => " :String\n");
				}
			}
			elsif ($data->isa('SWF::Element::ACTIONDATA::Boolean'))
			{
				writeint 2, $vftIndex{Boolean};
				align	 2, $ptrsize;
				writeint 2, $data->value;			# nLookup
			}
			# Apt0.16.00
			# added extra code for undef becuase swfc 0.34 defined separate undef value for undefined
			# but in Apt right now we treat them same and we use undefinedValue in both cases.
			elsif ( ($data->isa('SWF::Element::ACTIONDATA::NULL')) || ($data->isa('SWF::Element::ACTIONDATA::UNDEF')) )
			{
				writeint 2, $vftIndex{None};
				align	 2, $ptrsize;
				writeint 2, 0;						# pad
			}
			elsif ($data->isa('SWF::Element::ACTIONDATA::Register'))
			{
				writeint 2, $vftIndex{Register};
				align	 2, $ptrsize;
				writeint 2, $data->value;
			}
			else
			{
				die("TODO: don't support push of '" . ref($data) . "' yet");
			}

			$index++;
		}
	}
	
	# write out the sorted hash table (Only if user requests on the CommandLine)
	if ($constFileName)
	{	
		foreach my $key (sort keys %constFileStrings)
		{
			print CONST "$key : $constFileStrings{$key}\n";
		}
		close CONST;
	}
}

# this hash contains the shapes which referenced by hittest stage of any of the buttons
my %hittestshape;


# this function return 1 if a charactor(shape) is only referenced by hittest stage of any button by look into two hash
sub _forceToExportEmptyRU($)
{
	
	my $characterid = shift;
	# it's not a shape ever been used as a hit test region, return false
	unless (defined($hittestshape{$characterid}))
	{
		return 0;
	}

	# although it's a shape been used as hit test region, but it's also reference by other stuff, say : export table/ button's display stage/placeobject2 tag	
	# so we can't skip the geometry output
	if (defined($AptDerive::ObjectReallyGetReferenced{$characterid}))

	{
		return 0;
	}

	# otherwise, this shape can be skipped while exporting geometry	
	return 1;
}

# this function will go though all the buttons
# check each of its stages, if a shape is reference by the button in stage 1/2/4, add it into ObjectReallyGetReferenced
# if a shape is reference by the button in stage 8, add it into hittestshape
sub _calcHitTestShape($)
{
	my $characters = shift;
	my $nCharacters = scalar @$characters;
	for (my $charID = 0; $charID < $nCharacters; $charID++)
	{
		my $character = $characters->[$charID];
		if(defined($character))
		{
			if ($character->{type} eq 'button')
			{
				foreach my $br (@{$character->{buttonrecords}})
				{
					# any other stage
					if($br->{states} & 7)
					{
						$AptDerive::ObjectReallyGetReferenced{$br->{character}} = 1;
					}
					# the hit test stage
					if($br->{states} & 8)
					{
						$hittestshape{$br->{character}} = 1;
					}
				}
			}
		}
	}
}

sub dispOutFileInfo
{
    displayInfo "summary", "Total Textures (raw):$totalTextures Size(Bytes):$totalTextureSize \n" ;
    displayInfo "summary", "Total Sounds (raw)  :$totalSounds   Size(Bytes):$totalSoundSize \n" ;
    displayInfo "summary", "Total Chunks        :$totalChunks   Triangles  :$totalTris Lines :$totalLines\n" ;
    displayInfo "summary", "Total Constants     :$totalConstValues\n" ;
}

sub toFile
{
	my $filename = shift;
	my $endian = shift;
	my $ptrSize = shift;
	my $taglist = shift;
	my $characters = shift;
	my $movieData = shift;
	my $fps = shift;
	my $rect = shift;
	my $statistics = shift;
	my $constFileName = shift;
	my $swfversion = shift;			# --- added for AS/2 support
    my $writeDebuginformation = shift;
    my $timeStamp = shift;
    my $isDebugSwf = shift;
	
	my $movie = $movieData->{frames};
	my $imports = $movieData->{imports};
	my $exports = $movieData->{exports};

	my @textures;
	my %constPointers;
	
	_calcHitTestShape($characters);
	#print Dumper($taglist);

	if ($endian eq 'Network' || $endian eq 'big')
	{
		$intfmt = 'N';
	}
	elsif ($endian eq 'Vax' || $endian eq 'little')
	{
		$intfmt = 'V';
	}
	else
	{
		die "endianness '$endian' is not supported.\n";
	}
	
	if($ptrSize eq '8')
	{
		$ptrsize = "8";
	}

	open OUT, ">$filename.apt" or die "couldn't open '$filename.apt' for output";
	binmode(OUT, ":raw");

	$curPass = 0;

	# --- (rrv) enhancement for AS/2 support
	my $apttag;
	my $versionByte = $swfversion + 48;  # 48 is ASCII '0'
	
	# For SWF versions 1 through 9, if you open the AptData file (.apt) in a hex
	# editor, you'll see the version appear as a single-digit decimal value in ASCII.
	# But for SWF version 10 and up, since we only have one byte to work with, we store 
	# the version as a raw byte, plus 48. So it won't look nice in ASCII, but hey, this 
	# is a binary file. This scheme will work fine up to Flash version 207.

	$apttag = "Apt Data:1:" . chr($versionByte) . ":$ptrSize\032\0";
		
	#print ">>> SWF TAG($apttag) <<<\n";      # DC  Commented out Print Statement. (unneeded)
	
    setrawvalue($apttag);
	
	align 0, 4;


	lengthcheckstart("Total");

	$characters->[0]{type} = 'animation';
	$characters->[0]{movie} = $movie;

	my $nCharacters = scalar @$characters;
	foreach my $import (@$imports)
	{
		if ($import->{id} >= $nCharacters)
		{
			$nCharacters = $import->{id} + 1;
		}
	}


	# build the constant table. this has to be done beforehand, and not during the writing out
	# because we need to control the order that the constants are added in. why? because we
	# depend on the order in the resolve (and more specifically in the unresolve) at runtime
	# so that the current constant being resolved is always the next constant in the file.
	# this lets us not have to store an index along with the actual pointer that's stored.
	for (my $charID = 0; $charID < $nCharacters; $charID++)
	{
		my $character = $characters->[$charID];
		if (defined $character)
		{
			if ($character->{type} eq 'sprite' || $character->{type} eq 'animation')
			{
				addconstantsformovie $character->{movie};
			}
			elsif ($character->{type} eq 'button')
			{
				foreach my $ar (@{$character->{actionrecords}})
				{
					addactionsconstants $ar->{actions};
				}
			}
		}
	}


    $maxPassesNeeded = 3;
	for ($curPass = 1; $curPass <= $maxPassesNeeded; $curPass++)
	{
		lengthcheckstart("Imports");
		align 1, 4;
		if ($curPass == 1)
		{
			$constPointers{aImports} = getdatasize();
		}

		foreach my $import (@$imports)
		{
			# write our the AptImport array.
			writeptr 2, "main.aImports_${import}_szFile";
			writeptr 2, "main.aImports_${import}_szName";
			writeint 2, $import->{id};
			writeintptr 2, 0;								# file

			align 3, 4;
			fixupptr 3, "main.aImports_${import}_szFile";
			my $url = $import->{url};
			$url =~ s/\.swf$//i;
			writestring 3, $url;

			align 3, 4;
			fixupptr 3, "main.aImports_${import}_szName";
			writestring 3, $import->{name};

			if ($curPass == 3)
			{
				#print "import: '$url' : '$import->{name}' -> $import->{id}\n";
			}
		}
		lengthcheckstop("Imports");
	}

    $maxPassesNeeded = 3;
	for ($curPass = 1; $curPass <= $maxPassesNeeded ; $curPass++)
	{
		lengthcheckstart("Exports");
		align 1, $ptrsize;
		if ($curPass == 1)
		{
			$constPointers{aExports} = getdatasize();
		}

		foreach my $export (@$exports)
		{
			writeptr 2, "main.aExports_${export}_szName";
			writeint 2, $export->{id};

			align 3, 4;
			fixupptr 3, "main.aExports_${export}_szName";
			writestring 3, $export->{name};

			if ($curPass == 3)
			{
				#print "export: '$export->{name}' -> $export->{id}\n";
			}
		}
		lengthcheckstop("Exports");
	}

    # SWFC works on passes.  The maximum number of passes we can need was once marked here as 18.
    # But I'm pretty sure the maximum is actually 9.  So instead of figuring it out, I'll just
    # set it to the max needed for this block (5) and let the rest of the functions in this file
    # update the number if they need more passes
    $maxPassesNeeded = 5;
	for ($curPass = 1; $curPass <= $maxPassesNeeded ; $curPass++)
	{
		align 1, $ptrsize;
		if ($curPass == 1)
		{
			$constPointers{apCharacters} = getdatasize();
		}
		my $i = 0;
		for (my $charID = 0; $charID < $nCharacters; $charID++)
		{
			my $character = $characters->[$charID];

			unless (defined $character)
			{
				lengthcheckstart("char" . sprintf("%02d", $i) . " (undefined)");
				# if there's tagids that are skipped, don't point to anything here
				writeintptr 2, 0;
				lengthcheckstop("char" . sprintf("%02d", $i) . " (undefined)");
				$i++;
				next;
			}
			else
			{
				lengthcheckstart("char" . sprintf("%02d", $i) . " ($character->{type})");
			}

			#message 1, "[$curPass: $i / $#$characters]\r";
			writeptr 2, "apCharacter_${character}";	# apCharacter entry

			align 3, 4;
			fixupptr 3, "apCharacter_${character}";
			
			
            # We are about to write the character tag out to the file.  Before that happens,
            # store off a mapping of characters to file offsets so we can later figure 
            # out where this character is in the file
            if ($curPass == 3)
            {
                AptDebug::addCharacterOffsetMapping($character, getdatasize());
            }

			if ($character->{type} eq 'shape')
			{
				my $textID = -1;
				if ($curPass == 3)
				{
					$textID = _writeShape($character, $i,_forceToExportEmptyRU($charID));
				}
								
				writeint 3, 1;											# eType == AptCharacterType_Shape
				writeintptr 3, 0x09876543;								# pParentAnim
				writeshort 3, 0x0;									# m_nRef

				if($textID != -1)
				{
					writeshort 3, $textID;							# m_nRef
				}
				else
				{
					writeshort 3, 0x0;								# Texture ID
				}
				
				writeintptr 3, 0x0;									# m_pAnimFile
		
				# boundingRect
				if (defined($character->{boundingRect}))
				{
					writefloat 3, $character->{boundingRect}[0] / 20.0;
					writefloat 3, $character->{boundingRect}[1] / 20.0;
					writefloat 3, $character->{boundingRect}[2] / 20.0;
					writefloat 3, $character->{boundingRect}[3] / 20.0;
				}
				else
				{
					writefloat 3, 0.0;
					writefloat 3, 0.0;
					writefloat 3, 0.0;
					writefloat 3, 0.0;
				}

				writeintptr 3, $i;										# id of RU (zID)

				#if ($curPass == 3)
				#{
				#	_writeShape($character, $i,_forceToExportEmptyRU($charID));
				#}
			}
			elsif ($character->{type} eq 'morph')
			{
				writeint 3, 8;								# eType == AptCharacterType_Morph
				writeintptr 3, 0x09876543;					# pParentAnim
				writeint 3, 0x0;						# m_nRef
				writeintptr 3, 0x0;						# m_pAnimFile
				
				writeintptr 3, $character->{startCharID};	# id of Start RU
				writeintptr 3, $character->{endCharID};	# id of Start RU
			}
			elsif ($character->{type} eq 'sprite')
			{
				lengthcheckstart("sprite");
				writeint 3, 5;								# eType == AptCharacterType_Sprite
				writeintptr 3, 0x09876543;					# pParentAnim
				writeint 3, 0x0;						# m_nRef
				writeintptr 3, 0x0;						# m_pAnimFile
				
				lengthcheckstop("sprite");
				writemoviebase 3, $character->{movie};
				writemovieframes 4, $character->{movie};
			}
			elsif ($character->{type} eq 'animation')
			{
				if ($curPass == 3)
				{
					$constPointers{mainCharacter} = getdatasize();
				}

				lengthcheckstart("animation");
				writeint 3, 9;								# eType == AptCharacterType_Animation
				
				writeintptr 3, 0x09876543;					# pParentAnim
				
				writeint 3, 0x0;						# m_nRef
				writeintptr 3, 0x0;						# m_pAnimFile
				
				lengthcheckstop("animation");
				
				writemoviebase 3, $character->{movie};
				
				writemovieframes 4, $character->{movie};
				
				writeint 3, $nCharacters;					# nCharacters
				writeintptr 3, $constPointers{apCharacters};# apCharacters

				writeint 3, $rect->[2] / 20;				# nWidth
				writeint 3, $rect->[3] / 20;				# nHeight

				writeint 3, 1000 / $fps;					# nMillisecondsPerFrame

				writeint 3, $#$imports + 1;					# nImports
				writeintptr 3, $constPointers{aImports};	# aImports

				writeint 3, $#$exports + 1;					# nExports
				writeintptr 3, $constPointers{aExports};	# aExports

				writeint 3, 0;								# nCurrentConstantIndex
			}
			elsif ($character->{type} eq 'sound')
			{
				writeint 3, 6;							# eType == AptCharacterType_Sound
				writeintptr 3, 0x09876543;				# pParentAnim
				writeint 3, 0x0;					# m_nRef
				writeintptr 3, 0x0;					# m_pAnimFile
				
				writeintptr 3, $i;						# zID
			}
			elsif ($character->{type} eq 'texture')
			{
				writeint 3, 7;							# eType == AptCharacterType_Bitmap
				writeintptr 3, 0x09876543;				# pParentAnim
				writeint 3, 0x0;					# m_nRef
				writeintptr 3, 0x0;					# m_pAnimFile
				
				writeintptr 3, $character->{id};		# zID
				push @textures, $character->{tempfilename} if ($curPass == 1);
			}
			elsif ($character->{type} eq 'button')
			{
				writeint 3, 4;							# eType == AptCharacterType_Button
				writeintptr 3, 0x09876543;				# pParentAnim
				writeint 3, 0x0;					# m_nRef
				writeintptr 3, 0x0;					# m_pAnimFile
				
				writeint 3, $character->{ismenu};		# bIsMenu
				if($curPass == 3)
				{
					my @hitTestBoundingRect = _findTestBoundingRect($charID,$characters);
					writefloat 3, $hitTestBoundingRect[0];		# mHitTestBoundingRect
					writefloat 3, $hitTestBoundingRect[1];		# mHitTestBoundingRect
					writefloat 3, $hitTestBoundingRect[2];		# mHitTestBoundingRect
					writefloat 3, $hitTestBoundingRect[3];		# mHitTestBoundingRect
					
					my $HitTestVertsRef;
					my $HitTestIndicesRef;
					($HitTestVertsRef,$HitTestIndicesRef) = _getHitTestTriangles($charID,$characters);
					
					writeint 3,scalar(@$HitTestIndicesRef) /3 ;		# mHitTestTriangles
					writeint 3,scalar(@$HitTestVertsRef) /2 ;		# mHitTestVertexCount
					writeptr 3,"${charID}_HitTestVertexTable";		# mHitTestVertexTable
					writeptr 3,"${charID}_HitTestIndexTable";		# mHitTestIndexTable
				}
				writeint 3, ($#{$character->{buttonrecords}} + 1);	# nButtonRecords
				writeptr 3, "${character}_aButtonRecords";			# aButtonRecords
				writeint 3, ($#{$character->{actionrecords}} + 1);	# nActionRecords
				writeptr 3, "${character}_aActionConditions";		# aActionConditions
				lengthcheckstart("sound");
				if (defined($character->{sound}))
				{
					writeptr 3, "${character}_pButtonSound";		# pButtonSound
				}
				else
				{
					writeintptr 3, 0;									# pButtonSound
				}
				lengthcheckstop("sound");

				if($curPass == 4)
				{
					align 4, 4;
					fixupptr 4, "${charID}_HitTestVertexTable";		# mHitTestVertexTable
					
					my $HitTestVertsRef;
					my $HitTestIndicesRef;
					($HitTestVertsRef,$HitTestIndicesRef) = _getHitTestTriangles($charID,$characters);
					
					foreach my $coord (@$HitTestVertsRef)
					{
						writefloat 4, $coord;						# mHitTestCoodData[i]
					}
					
					align 4, 4;
					fixupptr 4, "${charID}_HitTestIndexTable";		# mHitTestVertexTable
					
					foreach my $theindex (@$HitTestIndicesRef)					
					{
						writeshort 4, $theindex;
					}
				}

				lengthcheckstart("records");
				align 4, 4;
				fixupptr 4, "${character}_aButtonRecords";			# aButtonRecords
				foreach my $br (@{$character->{buttonrecords}})
				{
					# Ensure each button is aligned properly
		            align 4, $ptrsize;
					
					writeint 4, $br->{states};						# eStates
					writeintptr 4, ($br->{character});				# pCharacter (index)
					writeint 4, $br->{layer};						# nLayer
					writematrix 4, $br->{matrix};					# matrix
					writecxform 4, $br->{cxform};					# cxform
				}
				lengthcheckstop("records");

				lengthcheckstart("conditions");
				align 4, 4;
				fixupptr 4, "${character}_aActionConditions";		# aActionConditions
				foreach my $ar (@{$character->{actionrecords}})
				{
					writeint 4,$ar->{condition};					# nConditions
					writeactions 4, "${character}_${ar}_actions", $ar->{actions};	# actions
				}
				lengthcheckstop("conditions");

				lengthcheckstart("sound");
				# only write if we have a sound associated with this button
				if (defined($character->{sound}))
				{
					align 4, 4;
					fixupptr 4, "${character}_pButtonSound";			# pButtonSound
					writeintptr 4, $character->{sound}{overuptoidle};
					writeintptr 4, $character->{sound}{idletooverup};
					writeintptr 4, $character->{sound}{overuptooverdown};
					writeintptr 4, $character->{sound}{overdowntooverup};
				}
				lengthcheckstop("sound");
			}
			elsif ($character->{type} eq 'font')
			{
				writeint 3, 3;										# eType == AptCharacterType_Font
				writeintptr 3, 0x09876543;							# pParentAnim
				writeint 3, 0x0;								# m_nRef
				writeintptr 3, 0x0;								# m_pAnimFile
				
				#writefixedstring 3, $character->{name}, 64;		# szName
				if (defined($character->{name}))
				{
					writeptr 3, "${character}_szName";
					align 4, $ptrsize;
					fixupptr 4, "${character}_szName";
					writestring 4, $character->{name};
				}
				else
				{
					writeintptr 3, 0;
				}

				if (defined($character->{glyphs}))
				{
					writeint 3, scalar @{$character->{glyphs}};		# nGlyphs
					writeptr 3, "${character}_apGlyphs";

					align 4, $ptrsize;
					fixupptr 4, "${character}_apGlyphs";
					foreach my $glyph (@{$character->{glyphs}})
					{
						writeintptr 4, $glyph;						# The glyphs array is defined as "AptCharacter **" thus need to write ptr size elements.
					}
				}
				else
				{
					writeint 3, 0;									# nGlyphs
					writeintptr 3, 0;							    # apGlyphs
				}
			}
			elsif ($character->{type} eq 'text')
			{
				writeint 3, 2;										# eType == AptCharacterType_Text
				writeintptr 3, 0x09876543;							# pParentAnim
				writeint 3, 0x0;								# m_nRef
				writeintptr 3, 0x0;								# m_pAnimFile
				
				writefloat 3, $character->{boundingRect}[0] / 20.0;	# rBound[0] (xmin)
				writefloat 3, $character->{boundingRect}[1] / 20.0;	# rBound[1] (ymin)
				writefloat 3, $character->{boundingRect}[2] / 20.0;	# rBound[2] (xmax)
				writefloat 3, $character->{boundingRect}[3] / 20.0;	# rBound[3] (ymax)
				# print ("$character->{boundingRect}[0], $character->{boundingRect}[1], $character->{boundingRect}[2], $character->{boundingRect}[3]\n") if $curPass = 3;
				writeint 3, $character->{fontid};					# nFontID
				writeint 3, $character->{alignment};				# eAlignment
				writecolour 3, $character->{colour};				# nColour
				writefloat 3, $character->{fontheight} / 20.0;		# fFontHeight
				writeint 3, $character->{readonly};					# bReadOnly
				writeint 3, $character->{multiline};				# bMultiLine
				writeint 3, $character->{wordwrap};					# bWordWrap
				writeptr 3, "${character}_szInitialText";			# szInitialText
				writeptr 3, "${character}_szVariable";				# szVariable

				align 4, 4;
				fixupptr 4, "${character}_szInitialText";
				writestring 4, $character->{initialtext};
				align 4, 4;
				fixupptr 4, "${character}_szVariable";
				writestring 4, $character->{variable};
				
				#writefixedstring 3, $character->{initialtext}, 256;	# szInitialText
				#writefixedstring 3, $character->{variable}, 64;		# szVariable
			}
			elsif ($character->{type} eq 'statictext')
			{
				writeint 3, 10;											# eType == AptCharacterType_StaticText
				writeintptr 3, 0x09876543;								# pParentAnim
				writeint 3, 0x0;									# m_nRef
				writeintptr 3, 0x0;									# m_pAnimFile
				
				writefloat 3, $character->{boundingRect}[0] / 20.0;	# rBounds[0] (xmin)
				writefloat 3, $character->{boundingRect}[1] / 20.0;	# rBounds[1] (ymin)
				writefloat 3, $character->{boundingRect}[2] / 20.0;	# rBounds[2] (xmax)
				writefloat 3, $character->{boundingRect}[3] / 20.0;	# rBounds[3] (ymax)
				writematrix 3, $character->{matrix};				# matrix
				writeint 3, scalar (@{$character->{records}});		# nFontRecords
				writeptr 3, "${character}_aRecords";				# aRecords

				align 4, 4;
				fixupptr 4, "${character}_aRecords";				# aRecords

				foreach my $record (@{$character->{records}})
				{
					writeint 4, $record->{font};					# nFontID
					writecxform 4, $record->{cxform};				# cxform
					writefloat 4, $record->{xoffset} / 20.0;		# fXOffset
					writefloat 4, $record->{yoffset} / 20.0;		# fYOffset
					writefloat 4, $record->{scale};					# fScale
					writeint 4, scalar (@{$record->{glyphs}});		# nGlyphs
					writeptr 4, "${character}_${record}_aGlyphs";	# aGlyphs

					align 5, 4;
					fixupptr 5, "${character}_${record}_aGlyphs";	# aGlyphs
					foreach my $glyph (@{$record->{glyphs}})
					{
						writeshort 5, $glyph->{index};				# nIndex
						writeshort 5, $glyph->{advance};			# nAdvance
					}
				}
			}
			elsif ($character->{type} eq 'video')
			{
				writeint 3, 12;								# eType == AptCharacterType_Video, to ignore the video
				writeintptr 3, 0x09876543;					# pParentAnim
				writeint 3, 0x0;						# m_nRef
				writeintptr 3, 0x0;						# m_pAnimFile				
			}
			else
			{
				message 0, "Unknown character: $character->{type}\n" if ($curPass == 1);
			}
			lengthcheckstop("char" . sprintf("%02d", $i) . " ($character->{type})");
			$i++;
		}
	}

	my $bufferData = getbuffer();
    syswrite OUT, $bufferData;
	close OUT;
    
	$curPass = 0;
	open OUT, ">$filename.const" or die "couldn't open '$filename.const' for output";
	binmode(OUT, ":raw");
	
    setrawvalue( "Apt1" );

    # Timestamp information
    my @timeStampBuffer;

    if ($isDebugSwf)
    {
        for my $byte (@{$timeStamp->{stamp}})
        {
            push @timeStampBuffer, $byte;
        }
    }
    else
    {
        for my $i (1..16)
        {
            push @timeStampBuffer, 0;
        }
    }

    $maxPassesNeeded = 1;
	for ($curPass = 1; $curPass <= $maxPassesNeeded; $curPass++)
	{
        for my $c (@timeStampBuffer)
        {
		    writechar 1, $c;
        }
	}

	$maxPassesNeeded = 1;
	for ($curPass = 1; $curPass <= $maxPassesNeeded; $curPass++)
	{
		writeintptr 1, $constPointers{mainCharacter};	# pMainCharacter
	}

	writeConstTable($constFileName);

    $bufferData = getbuffer();
    syswrite OUT, $bufferData;
    close OUT;

	lengthcheckstop("Total");
	
    if ($writeDebuginformation)
    {
        AptDebug::generateDebugFile($filename, $characters);
    }

	dumplengthstats if ($statistics);

	return 1;
}

# Return constant value by specified reference
sub getConstantByRef($)
{
    my $ref = shift;
    return $CONSTTABLE[$CONSTREF{$ref}];
}

1;
