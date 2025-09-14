# ***********************************************************************************************
#
#
#   \file    DebugInformation.pl
#
#   \brief   Debug information generator for ActionScript
#
#   (c) 2010 Electronic Arts Inc.
#
# ***********************************************************************************************

# Tell Perl that it can find additional includes in some nearby subdirectories.
# This only works when running the .pl file directly; if compiling with perlapp,
# you need to use --lib to tell it where to look.
use lib "SWF-File-0.36/lib";
use lib "Data-TemporaryBag-0.08/lib";

use strict;
use warnings;
use File::Basename;
use FindBin;
use SWF::BinStream;
use Data::Dumper;
use Getopt::Long;
use Log;

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief Base class of DebugInformation.
#	Do not use this class directly. If you want to add new
# debug information data type, please derive a new class,
# and call function RegisterType in DebugFile.
###########################################################
package DebugInformation;
our %TypeList = ();

###########################################################
# \brief Constructor for DebugInformation
###########################################################
sub new
{
	my $self = {};
	bless $self, "DebugInformation";
	$self->_init();
	return $self;
}

###########################################################
# \brief Initialization
###########################################################
sub _init
{
	my $self = shift;
	return $self;
}

###########################################################
# \brief Load from binary stream.
# \param	- Instance of SWF::BinStream::Read
###########################################################
sub load
{
	my $self = shift;
	my $stream = shift;
	return $self;
}

###########################################################
# \brief Save to binary stream
# \param	- Instance of SWF::BinStream::Write
###########################################################
sub save
{
	my $self = shift;
	my $stream = shift;
	return $self;
}

###########################################################
# \brief Read a string from binary stream
# \param	- Instance of SWF::BinStream::Read
# \return	- Return a string which is loaded from string.
# \note		- The string in binary stream must be end with
#			  '\0'
###########################################################
sub readString
{
	my $self = shift;
	my $stream = shift;
	
	my $c = 0;
	my $string = '';
	do
	{
		$c = $stream->get_SI8();
		if ( $c != 0 )
		{
			$string .= (pack 'c', $c);
		}
	} while($c != 0);
	
	return $string;
}

###########################################################
# \brief Write a string into binary stream
# \param	- Instance of SWF::BinStream::Write
# \param	- The string need to be written.
# \note		- The string in binary stream will be end with
#			  '\0'
###########################################################
sub writeString
{
	my $self = shift;
	my $stream = shift;
	my $string = shift;
	
	$stream->set_string($string);
	$stream->set_SI8(0);
	return $self;
}

###########################################################
# \brief Output debug information to console.(Debug purpose)
###########################################################
sub print
{
	my $self = shift;
	return $self;
}

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief Debug information for statement
#	Includes begin source code line, end source code line,
# begin offset, end offset and array of next statement index.
###########################################################
package DebugInformation::StatementInfo;
our @ISA = qw(DebugInformation);

###########################################################
# \brief Constructor for DebugInformation::StatementInfo
###########################################################
sub new ($)
{
	my ($type, $parent) = @_;
	my $self = {};
	$self->{Function} = $parent;
	bless $self, "DebugInformation::StatementInfo";
	$self->_init();
	return $self;
}

###########################################################
# \brief Initialization
###########################################################
sub _init
{
	my $self = shift;
	$self->SUPER::_init();
	return $self;
}

###########################################################
# \brief Load from binary stream
# \param	- Instance of SWF::BinStream::Read
###########################################################
sub load
{
	my $self = shift;
	my $stream = shift;
	$self->SUPER::load($stream);

	$self->{BeginLine} = $stream->get_UI32();
	$self->{EndLine} = $stream->get_UI32();

	$self->{BeginOffset} = $stream->get_UI32();
	$self->{EndOffset} = $stream->get_UI32();

	my @nextStatementList = ();
	$self->{NextStatementList} = \@nextStatementList;

	my $nextStatementCount = $stream->get_SI32();
	for my $i (1..$nextStatementCount)
	{
		my $offset = $stream->get_UI32();

		push @nextStatementList, $offset;
	}
	return $self;
}

###########################################################
# \brief Save to binary stream
# \param	- Instance of SWF::BinStream::Write
###########################################################
sub save
{
	my $self = shift;
	my $stream = shift;
    
    # Source code line
    $stream->set_UI32($self->{BeginLine});
    $stream->set_UI32($self->{EndLine});
    
    # Offset in bytecode stream
    $stream->set_UI32($self->{BeginOffset});
    $stream->set_UI32($self->{EndOffset});
    
    # Cout of statement info
    my $nextStatementCount = @{$self->{NextStatementList}};
    $stream->set_SI32($nextStatementCount);
    for my $offset (@{$self->{NextStatementList}})
    {
        $stream->set_UI32($offset);
    }
	return $self;
}

###########################################################
# \brief Output debug information to console.(Debug purpose)
###########################################################
sub print
{
	my $self = shift;

	print "^^^^^^^^^^^^^^^^^^^^^\n";
	print "Statement Begin Line:$self->{BeginLine}\n";
	print "Statement End Line:$self->{EndLine}\n";
	print "Statement Begin Offset:$self->{BeginOffset}\n";
	print "Statement End Offset:$self->{EndOffset}\n";
	print "Next statement index:@{$self->{NextStatementList}}\n";
	return $self;
}

###########################################################
# \brief Return begin line number
###########################################################
sub getBeginLine
{
	my $self = shift;
	return $self->{BeginLine};
}

###########################################################
# \brief Return end line number
###########################################################
sub getEndLine
{
	my $self = shift;
	return $self->{EndLine};
}

###########################################################
# \brief Return begin offset
###########################################################
sub getBeginOffset
{
	my $self = shift;
	return $self->{BeginOffset};
}

###########################################################
# \brief setBeginOffset
# \param - Offset to be set.
###########################################################
sub setBeginOffset
{
	my $self = shift;
    my $offset = shift;

    $self->{BeginOffset} = $offset;
	return $self;
}

###########################################################
# \brief Return end offset
###########################################################
sub getEndOffset
{
	my $self = shift;
	return $self->{EndOffset};
}

###########################################################
# \brief setEndOffset
# \param - Offset to be set.
###########################################################
sub setEndOffset
{
	my $self = shift;
    my $offset = shift;

    $self->{EndOffset} = $offset;
	return $self;
}

###########################################################
# \brief Return reference to the function which contains
# the statement.
###########################################################
sub getFunction()
{
    my $self = shift;
    return $self->{Function};
}

###########################################################
# \brief Add next statement offset
###########################################################
sub addNextStatementOffset($)
{
    my $self = shift;
    my $nextStatementOffset = shift;
    
    # Avoid duplication
    for my $offset (@{$self->{NextStatementList}})
    {
        if ( $offset == $nextStatementOffset )
        {
            return $self;
        }
    }
    
    push @{$self->{NextStatementList}}, $nextStatementOffset;
    return $self;
}

###########################################################
# \brief Return list next statement offset which are potential target for current statement.
###########################################################
sub getNextStatementOffsetList()
{
    my $self = shift;
    
    return $self->{NextStatementList};
}

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief Debug information for functions/methods
#	Include begin line, end line, begin offset, end offset
# and statement list
###########################################################
package DebugInformation::FunctionInfo;
our @ISA = qw(DebugInformation);

###########################################################
# \brief Constructor for DebugInformation::FunctionInfo
###########################################################
sub new (\@)
{
	my ($type, $parent) = @_;
	my $self = {};
	$self->{Parent} = $parent;
	bless $self, "DebugInformation::FunctionInfo";
	$self->_init();
	return $self;
}

###########################################################
# \brief Initialization
###########################################################
sub _init
{
	my $self = shift;
	$self->SUPER::_init();
	return $self;
}

###########################################################
# \brief Load from binary stream
# \param	- Instance of SWF::BinStream::Read
###########################################################
sub load
{
	my $self = shift;
	my $stream = shift;
	$self->SUPER::load($stream);

	$self->{Functionname} = $self->readString($stream);
	$self->{BeginLine} = $stream->get_UI32();
	$self->{EndLine} = $stream->get_UI32();
	$self->{BeginOffset} = $stream->get_UI32();
	$self->{EndOffset} = $stream->get_UI32();

	my $statementCount = $stream->get_SI32();
	die "Unknown error occurs when loading function information" if ($statementCount < 0);

	my @statementList = ();
	$self->{StatementInfoList} = \@statementList;

	for my $i (1..$statementCount)
	{
		my $statementInfo = DebugInformation::StatementInfo->new($self);
		$statementInfo->load($stream);

		push @statementList, $statementInfo;
	}
	return $self;
}

###########################################################
# \brief Save to binary stream
# \param	- Instance of SWF::BinStream::Write
###########################################################
sub save
{
	my $self = shift;
	my $stream = shift;
    
    # File name
    $self->writeString($stream, $self->{Functionname});
    
    # Source code line index
    $stream->set_UI32($self->{BeginLine});
    $stream->set_UI32($self->{EndLine});
    
    # Offset in bytecode stream
    $stream->set_UI32($self->{BeginOffset});
    $stream->set_UI32($self->{EndOffset});
    
    # Count of statement information
    my $statementCount = @{$self->{StatementInfoList}};
    $stream->set_SI32($statementCount);
    
    for my $statementInfo (@{$self->{StatementInfoList}})
    {
        $statementInfo->save($stream);
    }
	return $self;
}

###########################################################
# \brief Output debug information to console.(Debug purpose)
###########################################################
sub print
{
	my $self = shift;
	print "************************\n";
	print "Function Name: $self->{Functionname}\n";
	print "Begin Line: $self->{BeginLine}\n";
	print "End Line: $self->{EndLine}\n";
	print "Begin Offset: $self->{BeginOffset}\n";
	print "End Offset: $self->{EndOffset}\n";

	for my $statementInfo (@{$self->{StatementInfoList}})
	{
		$statementInfo->print();
	}

	print "************************\n";
	return $self;
}

###########################################################
# \brief Return function name
###########################################################
sub getName
{
	my $self = shift;
	return $self->{Functionname};
}

###########################################################
# \brief Return begin line number
###########################################################
sub getBeginLine
{
	my $self = shift;
	return $self->{BeginLine};
}

###########################################################
# \brief Return end line number
###########################################################
sub getEndLine
{
	my $self = shift;
	return $self->{EndLine};
}

###########################################################
# \brief Return begin offset
###########################################################
sub getBeginOffset
{
	my $self = shift;
	return $self->{BeginOffset};
}

###########################################################
# \brief Set begin offset
###########################################################
sub setBeginOffset
{
	my $self = shift;
    my $offset = shift;
	$self->{BeginOffset} = $offset;
    return $self;
}

###########################################################
# \brief Return end offset
###########################################################
sub getEndOffset
{
	my $self = shift;
	return $self->{EndOffset};
}

###########################################################
# \brief Set end offset
###########################################################
sub setEndOffset
{
	my $self = shift;
    my $offset = shift;
	$self->{EndOffset} = $offset;
    return $self;
}

###########################################################
# \brief Return statement information list
###########################################################
sub getStatementInfoList
{
	my $self = shift;
	return $self->{StatementInfoList};
}

###########################################################
# \brief Return InternalFile instance which contains the
# function.
###########################################################
sub getFile()
{
    my $self = shift;
    return $self->{Parent};
}

###########################################################
# \brief Find statement information by offset
###########################################################
sub getStatementInfoByOffset($)
{
	my $self = shift;
	my $offset = shift;

	for my $statementInfo (@{$self->{StatementInfoList}})
	{
		if ( $offset >= $statementInfo->getBeginOffset() && $offset <= $statementInfo->getEndOffset() )
		{
			return $statementInfo;
		}
	}
	return undef;
}

###########################################################
# \brief Update function offsets, internal file offsets
###########################################################
sub updateOffsets
{
    my $self = shift;
    my $statementCount = scalar(@{$self->{StatementInfoList}});
    if ($statementCount > 0)
    {
        $self->setBeginOffset($self->{StatementInfoList}->[0]->getBeginOffset());
        $self->setEndOffset($self->{StatementInfoList}->[$statementCount - 1]->getEndOffset());
    }
}

###########################################################
# \brief Find statement information index by offset
###########################################################
sub getStatementInfoIndexByOffset($)
{
    my $self = shift;
	my $offset = shift;

    my $count = @{$self->{StatementInfoList}} - 1;
	for my $i (0..$count)
	{
	    my $statementInfo = $self->{StatementInfoList}->[$i];
		if ( $offset >= $statementInfo->getBeginOffset() && $offset <= $statementInfo->getEndOffset() )
		{
			return $i;
		}
	}
	return undef;
}

###########################################################
# \brief Find index of specified statement information
###########################################################
sub getStatementInfoIndex($)
{
    my $self = shift;
	my $targetStatement = shift;

    my $count = @{$self->{StatementInfoList}} - 1;
	for my $i (0..$count)
	{
	    my $statementInfo = $self->{StatementInfoList}->[$i];
		if ( $statementInfo == $targetStatement )
		{
			return $i;
		}
	}
	return undef;
}

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief Debug information for internal file.
###########################################################
package DebugInformation::InternalFile;
our @ISA = qw(DebugInformation);

###########################################################
# \brief Constructor for DebugInformation::InternalFile
###########################################################
sub new (\@)
{
	my ($type, $parent) = @_;
	my $self = {};
	$self->{Parent} = $parent;
	bless $self, "DebugInformation::InternalFile";
	$self->_init();
	return $self;
}

###########################################################
# \brief Initialization
###########################################################
sub _init
{
	my $self = shift;
	$self->SUPER::_init();
	return $self;
}

###########################################################
# \brief Load from binary stream
# \param	- Instance of SWF::BinStream::Read
###########################################################
sub load
{
	my $self = shift;
	my $stream = shift;
	$self->SUPER::load($stream);

	$self->{FileId} = $stream->get_UI32();
	$self->{FileName} = $self->readString($stream);
	$self->{SourceCode} = $self->readString($stream);
	$self->{BeginOffset} = $stream->get_UI32();
	$self->{EndOffset} = $stream->get_UI32();

	# Function information list
	my @functionList = ();
	$self->{FunctionInfoList} = \@functionList;

	my $functionCount = $stream->get_SI32();
	for my $i (1..$functionCount)
	{
		my $functionInfo = DebugInformation::FunctionInfo->new($self);
		$functionInfo->load($stream);

		push @functionList, $functionInfo;
	}
	return $self;
}

###########################################################
# \brief Save to binary stream
# \param	- Instance of SWF::BinStream::Write
###########################################################
sub save
{
	my $self = shift;
	my $stream = shift;
    
    # File ID
    $stream->set_UI32($self->{FileId});
    
    # File name
    $self->writeString($stream, $self->{FileName});
    
    # Source code
    $self->writeString($stream, $self->{SourceCode});
    
    # Offset in bytecode stream
    $stream->set_UI32($self->{BeginOffset});
    $stream->set_UI32($self->{EndOffset});
    
    my $functionCount = @{$self->{FunctionInfoList}};
    $stream->set_SI32($functionCount);
    
    for my $functionInfo (@{$self->{FunctionInfoList}})
    {
        $functionInfo->save($stream);
    }
	return $self;
}

###########################################################
# \brief Output debug information to console.(Debug purpose)
###########################################################
sub print
{
	my $self = shift;
	print "----------------------------------\n";
	print "File Id: $self->{FileId}\n";
	print "Begin Offset: $self->{BeginOffset}\n";
	print "End Offset: $self->{EndOffset}\n";
	print "File Name: $self->{FileName}\n";

	for my $functionInfo (@{$self->{FunctionInfoList}})
	{
		$functionInfo->print();
	}

	print "----------------------------------\n";
	return $self;
}

###########################################################
# \brief Return file ID
###########################################################
sub getFileId
{
	my $self = shift;
	return $self->{FileId};
}

###########################################################
# \brief Return file name
###########################################################
sub getFileName
{
	my $self = shift;
	return $self->{FileName};
}

###########################################################
# \brief Return source code of the file
###########################################################
sub getSourceCode
{
	my $self = shift;
	return $self->{SourceCode};
}

###########################################################
# \brief Return begin offset
###########################################################
sub getBeginOffset
{
	my $self = shift;
	return $self->{BeginOffset};
}

###########################################################
# \brief Set begin offset
###########################################################
sub setBeginOffset
{
	my $self = shift;
    my $offset = shift;
	$self->{BeginOffset} = $offset;
    return $self;
}

###########################################################
# \brief Return end offset
###########################################################
sub getEndOffset
{
	my $self = shift;
	return $self->{EndOffset};
}

###########################################################
# \brief Set end offset
###########################################################
sub setEndOffset
{
	my $self = shift;
    my $offset = shift;
	$self->{EndOffset} = $offset;
    return $self;
}

###########################################################
# \brief Return function information list
###########################################################
sub getFunctionInfoList
{
	my $self = shift;
	return $self->{FunctionInfoList};
}

###########################################################
# \brief Find function information by offset
###########################################################
sub getFunctionInfoByOffset($)
{
	my $self = shift;
	my $offset = shift;

	my $targetBeginOffset = 0;
	my $targetEndOffset = 0xFFFFFFFF;

	for my $functionInfo (@{$self->{FunctionInfoList}})
	{
		my $functionBeginOffset = $functionInfo->getBeginOffset();
		my $functionEndOffset = $functionInfo->getEndOffset();

		if ( $functionBeginOffset <= $offset && $functionEndOffset >= $offset )
        {
            if ( $functionBeginOffset > $targetBeginOffset )
            {
                $targetBeginOffset = $functionBeginOffset;
            }

            if ( $functionEndOffset < $targetEndOffset )
            {
                $targetEndOffset = $functionEndOffset;
            }
        }
	}

	for my $functionInfo (@{$self->{FunctionInfoList}})
	{
		if ($functionInfo->getBeginOffset() == $targetBeginOffset && $functionInfo->getEndOffset() == $targetEndOffset)
		{
			return $functionInfo;
		}
	}
	return undef;
}

###########################################################
# \brief Update function offsets, internal file offsets
###########################################################
sub updateOffsets
{
    my $self = shift;
    my $minOffset  = 0xFFFFFFFF;
    my $maxOffset  = 0;
    for my $functionInfo (@{$self->{FunctionInfoList}})
    {
        $functionInfo->updateOffsets();

        if ( $functionInfo->getBeginOffset() < $minOffset )
        {
            $minOffset = $functionInfo->getBeginOffset();
        }

        if ( $functionInfo->getEndOffset() > $maxOffset )
        {
            $maxOffset = $functionInfo->getEndOffset();
        }
    }

    if ($minOffset < 0xFFFFFFFF)
    {
        $self->setBeginOffset($minOffset);
        $self->setEndOffset($maxOffset);
    }
    else
    {
        $self->setBeginOffset(0xFFFFFFFF);
        $self->setEndOffset(0xFFFFFFFF);
    }
}

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief Debug information for assembly instruction
#	Include code line, offset, next instruction offset
###########################################################
package DebugInformation::AssemblyInstruction;
our @ISA = qw(DebugInformation);

###########################################################
# \brief Constructor for DebugInformation::AssemblyInstruction
###########################################################
sub new ($)
{
	my ($type, $parent) = @_;
	my $self = {};
	$self->{File} = $parent;
	bless $self, "DebugInformation::AssemblyInstruction";
	$self->_init();
	return $self;
}

###########################################################
# \brief Initialization
###########################################################
sub _init
{
	my $self = shift;
	$self->SUPER::_init();
    
    my @nextInstructionList = ();
	$self->{NextInstructionList} = \@nextInstructionList;
	return $self;
}

###########################################################
# \brief Load from binary stream
# \param	- Instance of SWF::BinStream::Read
###########################################################
sub load
{
	my $self = shift;
	my $stream = shift;
	$self->SUPER::load($stream);
	
	$self->{Line} = $stream->get_UI32($stream);
	$self->{Offset} = $stream->get_UI32($stream);
	
	# Next instruction offset list
	my $instructionCount = $stream->get_SI32();
	for my $i (1..$instructionCount)
	{
		my $offset = $stream->get_UI32($stream);

		push @{$self->{NextInstructionList}}, $offset;
	}

	return $self;
}

###########################################################
# \brief Save to binary stream
# \param	- Instance of SWF::BinStream::Write
###########################################################
sub save
{
	my $self = shift;
	my $stream = shift;
    
    # Line index in assembly code
    $stream->set_UI32($self->{Line});
    
    # Offset of the instruction in bytecode stream
    $stream->set_UI32($self->{Offset});
    
    # Count of next instructions
    my $instructionCount = @{$self->{NextInstructionList}};
	$stream->set_SI32($instructionCount);
    
    # Next instructions offset list
    for my $offset (@{$self->{NextInstructionList}})
    {
        $stream->set_UI32($offset);
    }
	
	return $self;
}

###########################################################
# \brief Output debug information to console.(Debug purpose)
###########################################################
sub print
{
	my $self = shift;
    
    print "Line:$self->{Line}\t Offset:$self->{Offset}\n";
	print "NextInstructions: @{$self->{NextInstructionList}}\n";
	return $self;
}

###########################################################
# \brief Set assembly code line of the instruction
# \param	- Line number
###########################################################
sub setLine($)
{
    my $self = shift;
    my $line = shift;
    
    $self->{Line} = $line;
    return $self;
}

###########################################################
# \brief Return assembly code line of the instruction
###########################################################
sub getLine()
{
    my $self = shift;
    return $self->{Line};
}

###########################################################
# \brief Set offset of the instruction in bytecode stream
# \param	- Line number
###########################################################
sub setOffset($)
{
    my $self = shift;
    my $offset = shift;
    
    $self->{Offset} = $offset;
    return $self;
}

###########################################################
# \brief Return offset of the instruction in bytecode stream
###########################################################
sub getOffset()
{
    my $self = shift;
    return $self->{Offset};
}

###########################################################
# \brief Add next instruction offset to current instruction
###########################################################
sub addNextInstructionByOffset($)
{
    my $self = shift;
    my $offset = shift;
    for my $i (@{$self->{NextInstructionList}})
    {
        if ( $offset == $i )
        {
            return $self;
        }
    }
    push @{$self->{NextInstructionList}}, $offset;
    return $self;
}

###########################################################
# \brief Return next instruction offset list which are potential target of current instruction
###########################################################
sub getNextInstructinOffsetList()
{
    my $self = shift;
    
    return $self->{NextInstructionList};
}

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief Debug information for assembly code file.
#	Include assembly code, instructions
###########################################################
package DebugInformation::AssemblyFile;
our @ISA = qw(DebugInformation);

###########################################################
# \brief Constructor for DebugInformation::AssemblyFile
###########################################################
sub new ($)
{
	my ($type, $parent) = @_;
	my $self = {};
	$self->{Parent} = $parent;
	bless $self, "DebugInformation::AssemblyFile";
	$self->_init();
	return $self;
}

###########################################################
# \brief Initialization
###########################################################
sub _init
{
	my $self = shift;
	$self->SUPER::_init();
	return $self;
}

###########################################################
# \brief Load from binary stream
# \param	- Instance of SWF::BinStream::Read
###########################################################
sub load
{
	my $self = shift;
	my $stream = shift;
	$self->SUPER::load($stream);
	
	$self->{AssemblyCode} = $self->readString($stream);
	
	# Assembly code lines
	my @assemblyInstructionList = ();
	$self->{InstructionList} = \@assemblyInstructionList;

	my $instructionCount = $stream->get_SI32();
	for my $i (1..$instructionCount)
	{
		my $instruction = DebugInformation::AssemblyInstruction->new($self);
		$instruction->load($stream);

		push @assemblyInstructionList, $instruction;
	}

	return $self;
}

###########################################################
# \brief Save to binary stream
# \param	- Instance of SWF::BinStream::Write
###########################################################
sub save
{
	my $self = shift;
	my $stream = shift;
    
    # Assembly code
    $self->writeString($stream, $self->{AssemblyCode});
    
    # Count of instructions
    my $instructionCount = @{$self->{InstructionList}};
    $stream->set_SI32($instructionCount);
    
    for my $instructionInfo (@{$self->{InstructionList}})
    {
        $instructionInfo->save($stream);
    }
    
	return $self;
}

###########################################################
# \brief Return assembly code
###########################################################
sub getAssemblyCode()
{
    my $self = shift;
    return $self->{AssemblyCode}
}

###########################################################
# \brief Set assembly code of the file
###########################################################
sub setAssemblyCode($)
{
    my $self = shift;
    my $assemblyCode = shift;
    $self->{AssemblyCode} = $assemblyCode;
    return $self;
}

###########################################################
# \brief Output debug information to console.(Debug purpose)
###########################################################
sub print
{
	my $self = shift;
	
	print "Assembly Code:\n" . $self->{AssemblyCode} . "\n";
	
	for my $instructionInfo (@{$self->{InstructionList}})
	{
		$instructionInfo->print();
	}
	return $self;
}

###########################################################
# \brief Add instruction information instance to current assembly file.
# \param    Instance of instruction information
###########################################################
sub addInstructionInfo($)
{
    my $self = shift;
    my $instructionInfo = shift;

    # We know these are being added in order, so the list will remain sorted
    push @{$self->{InstructionList}}, $instructionInfo;
    return $self;
}

###########################################################
# \brief Find instruction information by the specified offset
# \param    Offset to be find
###########################################################
sub getInstructionByOffset($)
{
    my $self = shift;
    my $offset = shift;

    my $index = $self->findOffsetIndex($offset);
    if ($index >= 0)
    {
        return @{$self->{InstructionList}}[$index];
    }
    return undef;
}

###########################################################
# \brief   Searches for an instrustion with the given offset
#          in the instruction list
# \param   The offset to search for
# \returns The index of the instruction containing the
#          offset, or the bitwise complement of the
#          insertion index if the offset is not found
###########################################################
sub findOffsetIndex($)
{
    my $self = shift;
    my $find = shift;

    # Get a reference to the instruction array
    my $list = \@{$self->{InstructionList}};

    # Do a binary search for the offset
    my ($lower, $upper) = (0, scalar(@$list) - 1);
    my $i;
    while ($lower <= $upper)
    {
        $i = int(($lower + $upper) / 2);
        if (@$list[$i]->{'Offset'} < $find)
        {
            $lower = $i + 1; 
        }
        elsif (@$list[$i]->{'Offset'} > $find)
        {
            $upper = $i - 1;
        }
        else
        {
            return $i;
        }
    }
    return ~$lower;
}

###########################################################
# \brief Return instructions list included in the assembly file.
###########################################################
sub getInstructionList()
{
    my $self = shift;
    
    return $self->{InstructionList};
}

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief Debug information for source code file.
#	Include file index, file name and file content.
###########################################################
package DebugFile;

###########################################################
# \brief Constructor for DebugInformation::SourceFile
###########################################################
sub new
{
	my $self = {};
	bless $self, "DebugFile";
	$self->_init();
	return $self;
}

###########################################################
# \brief Initialization
###########################################################
sub _init
{
	my $self = shift;
	return $self;
}

###########################################################
# \brief Load from binary stream
# \param	- Instance of SWF::BinStream::Read
###########################################################
sub load
{
	my $self = shift;
	my $stream = shift;
	my @header = ();
	$self->{Header} = \@header;
	for my $i (1..4)
	{
		push @header, $stream->get_UI8();
	}
	# Header is "APD" + version
	# Version is 1
	die "Invalid pre-build debug information file!" if ($header[0] != 0x41 || $header[1] != 0x50 || $header[2] != 0x44 || $header[3] != 1);
	
	# Timestamp is 16 bytes which is generated by Flash IDE
	my @timeStamp = ();
	$self->{TimeStamp} = \@timeStamp;
	
	for my $i (1..16)
	{
		push @timeStamp, $stream->get_UI8();
	}

	my $internalFileCount = $stream->get_SI32();

	my @internalFileList = ();
	for my $i (1..$internalFileCount)
	{
		my $internalFile = DebugInformation::InternalFile->new($self);
		$internalFile->load($stream);
		push @internalFileList, $internalFile;
	}

	$self->{InternalFileList} = \@internalFileList;
	
	my $assemblyFile = DebugInformation::AssemblyFile->new($self);
	$assemblyFile->load($stream);
	
	$self->{AssemblyFile} = $assemblyFile;
	return $self;
}

###########################################################
# \brief Save to binary stream
# \param	- Instance of SWF::BinStream::Write
###########################################################
sub save
{
	my $self = shift;
	my $stream = shift;
    
    # Header
	for my $i (@{$self->{Header}})
	{
		$stream->set_UI8($i);
	}
	
	# Timestamp
	for my $i (@{$self->{TimeStamp}})
	{
		$stream->set_UI8($i);
	}
    
    my $internalFileCount = @{$self->{InternalFileList}};
    
    # Count of internal files
	$stream->set_SI32($internalFileCount);
	
    for my $internalFile (@{$self->{InternalFileList}})
    {
        $internalFile->save($stream);
    }
    
    $self->{AssemblyFile}->save($stream);
	return $self;
}

###########################################################
# \brief Output debug information to console.(Debug purpose)
###########################################################
sub print
{
	my $self = shift;
	
	print "Timestamp";
	for my $i (@{$self->{TimeStamp}})
	{
		print $i . ","
	}
	print "\n";

	for my $internalFile (@{$self->{InternalFileList}})
	{
		$internalFile->print();
	}
	
	$self->{AssemblyFile}->print();
	return $self;
}

###########################################################
# \brief Find function information by offset
# \param	- Offset to be searched.
###########################################################
sub getFunctionInfoByOffset($)
{
	my $self = shift;
	my $offset = shift;
    my $internalFile = undef;

	for my $i (@{$self->{InternalFileList}})
	{
		if ($i->getBeginOffset() <= $offset && $i->getEndOffset() >= $offset)
        {
            $internalFile = $i;
            last;
		}
	}

    my $functionInfo = undef;
    if ( defined($internalFile) )
    {
        $functionInfo = $internalFile->getFunctionInfoByOffset($offset);
    }
    return $functionInfo;
}

###########################################################
# \brief Find statement information by offset
# \param	- Offset to be searched.
###########################################################
sub getStatementInfoByOffset($)
{
	my $self = shift;
	my $offset = shift;

    my $functionInfo = $self->getFunctionInfoByOffset($offset);
    if ( defined($functionInfo) )
    {
        return $functionInfo->getStatementInfoByOffset($offset);
    }
    return undef;
}

###########################################################
# \brief Update function offsets, internal file offsets
###########################################################
sub updateOffsets
{
    my $self = shift;

    for my $internalFile (@{$self->{InternalFileList}})
	{
		$internalFile->updateOffsets();
	}
}

###########################################################
# \brief Return internal file list
###########################################################
sub getFileList
{
    my $self = shift;

    die "Can't access DebugFile::getFileList() as static function!" if !defined($self);

    return $self->{InternalFileList};
}

###########################################################
# \brief Return assembly file information instance
###########################################################
sub getAssemblyFile
{
    my $self = shift;

    die "Can't access DebugFile::getAssemblyFile() as static function!" if !defined($self);

    return $self->{AssemblyFile};
}

###########################################################
# \brief Find instruction information by the specified offset
# \param    Offset to be find
###########################################################
sub getInstructionByOffset($)
{
    my $self = shift;
    my $offset = shift;
    
    return $self->{AssemblyFile}->getInstructionByOffset($offset);
}

#**************************************************************************************************************
#**************************************************************************************************************
###########################################################
# \brief DebugFile testing code wrapper
###########################################################
package DebugFile::UnitTest;

sub Test
{
	my $inputFileName = shift;
	my $outputFileName = shift;
	# Read file
	my $file;
	open($file, $inputFileName) || die "Can't open $inputFileName: $!";
	binmode($file);

	# Get file size
	seek($file, 0, 2);
	my $fileSize = tell($file);
	seek($file, 0, 0);

	# Load file into stream
	my $buffer = '';
	read($file, $buffer, $fileSize);
	my $stream = SWF::BinStream::Read->new('', sub{ die "The stream ran short by $_[0] bytes."});
	$stream->add_stream($buffer);
	close($file);

	# Load debug information from stream
	my $debugFile = DebugFile->new();
	$debugFile->load($stream);

    $debugFile->print();

	# Write file
	open($file, "> $outputFileName") or die "Can't open $outputFileName: $!";
	binmode($file);
	$stream = SWF::BinStream::Write->new();
	#$stream->autoflush(1000, sub {print $file $_[1]});
	$debugFile->save($stream);
	print $file $stream->flush_stream;
	close $file;
}

###########################################################
# Testing code, remove it if you don't need it
###########################################################
#DebugFile::UnitTest::Test("test.apd", "tt.apd");