use lib "SWF-File-0.36/lib";
use lib "Data-TemporaryBag-0.08/lib";

package AptDebug;
use strict;
use warnings;
use AptMessage ':all';
use Data::Dumper;
use Carp;
use AptCompiler ':all';
use AptDerive;
use DebugInformation;
use AptOutput;
use SWF::BinStream;

# For a bunch of stuff we are going to write to the Apt file, we want to track where we wrote it.
# This will allow us to output some debug information later about it
my %fileOffsetFromAction;       # Maps Actions to file offsets
my %fileOffsetFromControl;      # Maps Controls to file offsets
my %fileOffsetFromCharacter;    # Maps characters to file offsets
my @offsetList;                 # Maps Actions to file offsets
my %originalToFinalOffsetMap;   # Maps original offsets to final offsets
my @instructionsList;           # Assembly instruction list
my $currentAssemblyLine = 1;    # Current line No. of assembly code
my %alwaysJumpStatementList;    # Hash table to save the statement information which is always jump to other target statement.
my %alwaysJumpInstructionList;  # Hash table to save the instruction information which is always jump to other target instruction.

# Buffer we may write out to a debug file
my $debugFileBuffer;

# Debug information file
my $debugFile;

# Simple action to name hash table
my %simpleActionPrintText =
(
    0x00 => "end",
    0x04 => "nextFrame",
    0x05 => "prevFrame",
    0x06 => "play",
    0x07 => "stop",
    0x08 => "toggleQuality",
    0x09 => "stopSounds",
    0x0A => "add",
    0x0B => "substract",
    0x0C => "multiply",
    0x0D => "divide",
    0x0E => "equals",
    0x0F => "lessThan",
    0x10 => "and",
    0x11 => "or",
    0x12 => "not",
    0x13 => "stringEquals",
    0x14 => "stringLength",
    0x15 => "subString",
    0x17 => "pop",
    0x18 => "toInteger",
    0x1C => "getVariable",
    0x1D => "setVariable",
    0x20 => "setTarget2",
    0x21 => "stringAdd",
    0x22 => "getProperty",
    0x23 => "setProperty",
    0x24 => "cloneSprite",
    0x25 => "removeSprite",
    0x26 => "trace",
    0x27 => "startDragMovie",
    0x28 => "stopDragMovie",
    0x29 => "stringLessThan",
    0x2A => "throw",
    0x2B => "castOp",
    0x2C => "implementsOp",
    0x30 => "random",
    0x31 => "mbLength",
    0x32 => "charToAscii",
    0x33 => "asciiToChar",
    0x34 => "getTimer",
    0x35 => "mbSubString",
    0x36 => "mbCharToAscii",
    0x37 => "mbAsciiToChar",
    0x3A => "delete",
    0x3B => "delete2",
    0x3C => "defineLocal",
    0x3D => "callFunction",
    0x3E => "return",
    0x3F => "modulo",
    0x40 => "newObject",
    0x41 => "defineLocal2",
    0x42 => "initArray",
    0x43 => "initObject",
    0x44 => "typeOf",
    0x45 => "targetPath",
    0x46 => "enumerate",
    0x47 => "add2",
    0x48 => "lessThan2",
    0x49 => "equals2",
    0x4A => "toNumber",
    0x4B => "toString",
    0x4C => "pushDuplicate",
    0x4D => "stackSwap",
    0x4E => "getMember",
    0x4F => "setMember",
    0x50 => "increment",
    0x51 => "decrement",
    0x52 => "callMethod",
    0x53 => "newMethod",
    0x54 => "instanceOf",
    0x55 => "enumerate2",
    0x56 => "pushThis",
    0x58 => "pushGlobal",
    0x59 => "push0",
    0x5A => "push1",
    0x5B => "callFuncAndPop",
    0x5C => "callFuncSetVar",
    0x5D => "callMethodPop",
    0x5E => "callMethodSetVar",
    0x60 => "bitAnd",
    0x61 => "bitOr",
    0x62 => "bitXor",
    0x63 => "bitLShift",
    0x64 => "bitRShift",
    0x65 => "bitURShift",
    0x66 => "strictEquals",
    0x67 => "greater",
    0x69 => "extends",
    0x70 => "pushThisVariable",
    0x71 => "pushGlobalVariable",
    0x72 => "pushZeroSetVar",
    0x73 => "pushTrue",
    0x74 => "pushFalse",
    0x75 => "pushNULL",
    0x76 => "pushUndefined",
    0x77 => "traceStart",
    0x81 => "gotoFrame",
    0x83 => "getUrl",
    0x87 => "storeRegister",
    0x88 => "defineDictionary",
    0x8A => "waitForFrame",
    0x8B => "setTarget",
    0x8C => "gotoLabel",
    0x8E => "defineFunction2",
    0x8F => "try",
    0x94 => "with",
    0x96 => "push",
    0x99 => "branchAlways",
    0x9A => "getUrl2",
    0x9B => "defineFunction",
    0x9D => "branchIfTrue",
    0x9E => "callFrame",
    0x9F => "gotoFrame2",
    0xA1 => "pushString",
    0xA2 => "pushStringDictByte",
    0xA3 => "pushStringDictWord",
    0xA4 => "pushStringGetVar",
    0xA5 => "pushStringGetMember",
    0xA6 => "pushStringSetVar",
    0xA7 => "pushStringSetMember",
    0xAE => "stringDictByteGetVar",
    0xAF => "stringDictByteGetMember",
    0xB0 => "dictCallFuncPop",
    0xB1 => "dictCallFuncSetVar",
    0xB2 => "dictCallMethodPop",
    0xB3 => "dictCallMethodSetVar",
    0xB4 => "pushFloat",
    0xB5 => "pushByte",
    0xB6 => "pushWord",
    0xB7 => "pushDWord",
    0xB8 => "branchIfFalse",
    0xB9 => "pushRegister"
);

# This function removes characters from constants that mess up debugging output
sub sanitizeConstant
{
    my $str = shift;
    $str =~ s/\n/\\n/g;
    $str =~ s/\t/\\t/g;
    $str =~ s/\r/\\r/g;
    return $str;
}

# \brief Create a simple instruction information
# \param An action instance
# \param Next instruction offset [Optional]
# \return Return the created instruction information instance.
sub createInstructionInfo
{
    my $action = shift;
    
    my $nextInstruction = undef;
    
    if (@_ > 0)
    {
        $nextInstruction = shift;
    }
    
    if ( !defined($debugFile) )
    {
        return;
    }
    
    my $instructionInfo = DebugInformation::AssemblyInstruction->new($debugFile->getAssemblyFile());
    $instructionInfo->setLine($currentAssemblyLine);
    $instructionInfo->setOffset($originalToFinalOffsetMap{$action->OriginalStreamOffset});
    if ( defined($nextInstruction) )
    {
        $instructionInfo->addNextInstructionByOffset($nextInstruction);
    }
    
    push @instructionsList, $instructionInfo;
    
    $debugFile->getAssemblyFile()->addInstructionInfo($instructionInfo);
    
    return $instructionInfo;
}

# Output simple action as assembly code
sub simpleActionPrintFunction
{
    my $action = shift;
    my $tag = $action->Tag->value;

    $debugFileBuffer .= $simpleActionPrintText{$tag};
    
    createInstructionInfo($action);
}

# Output ActionConstantPool action as assembly code
sub constantPoolPrintFunction
{
    my $action = shift;
    my @dataList = @{$action->ConstantPool};
    $debugFileBuffer .= "constants ";
    my $i = 0;
    my $dataSize = $#dataList;

    foreach my $data (@dataList)
    {
        my $str = AptOutput::_uniqueNameForData($action, $i, $data);
        # prevent constants from format problems
        $debugFileBuffer .= "'" . sanitizeConstant(AptOutput::getConstantByRef($str)) . "'";

        if ($i < $dataSize)
        {
            $debugFileBuffer .= ", ";
        }
        $i++;
    }
    createInstructionInfo($action);
}

# Output ActionPush action as assembly code
sub pushPrintFunction
{
    my $action = shift;

    my @dataList = @{$action->DataList};
    $debugFileBuffer .= "push ";

    my $i = 0;
    my $dataSize = $#dataList;
    foreach my $data (@dataList)
    {
        if ( $data->isa("SWF::Element::ACTIONDATA::NULL") )
        {
            $debugFileBuffer .= "<null>";
        }
        elsif ( $data->isa("SWF::Element::ACTIONDATA::UNDEF") )
        {
            $debugFileBuffer .= "<undefined>";
        }
        else
        {
            my $str = AptOutput::_uniqueNameForData($action, $i, $data);
            # prevent constants from format problems
            $debugFileBuffer .= "'" . sanitizeConstant(AptOutput::getConstantByRef($str)) . "'";
        }

        if ($i < $dataSize)
        {
            $debugFileBuffer .= ", ";
        }
        $i++;
    }
    createInstructionInfo($action);
}

# Output ActionGetUrl2 action as assembly code
sub getUrl2PrintFunction
{
    my $action = shift;

    my $method = $action->Method;
    $debugFileBuffer .= "getUrl2 (";
    if ( $method eq 0x00 )
    {
        $debugFileBuffer .=  "no method";
    }
    elsif ( $method eq 0x01 )
    {
        $debugFileBuffer .=  "GET";
    }
    elsif ( $method eq 0x02 )
    {
        $debugFileBuffer .=  "POST";
    }
    elsif ( $method eq 0x40 )
    {
        $debugFileBuffer .=  "loadMovie";
    }
    elsif ( $method eq 0x41 )
    {
        $debugFileBuffer .=  "loadMovie GET";
    }
    elsif ( $method eq 0x42 )
    {
        $debugFileBuffer .=  "loadMovie POST";
    }
    elsif ( $method eq 0x80 )
    {
        $debugFileBuffer .=  "loadVariablesNum";
    }
    elsif ( $method eq 0x81 )
    {
        $debugFileBuffer .=  "loadVariablesNum GET";
    }
    elsif ( $method eq 0x82 )
    {
        $debugFileBuffer .=  "loadVariablesNum POST";
    }
    elsif ( $method eq 0xC0 )
    {
        $debugFileBuffer .=  "loadVariables";
    }
    elsif ( $method eq 0xC1 )
    {
        $debugFileBuffer .=  "loadVariables GET";
    }
    elsif ( $method eq 0xC2 )
    {
        $debugFileBuffer .=  "loadVariables POST";
    }
    else
    {
        $debugFileBuffer .=  "unknown method: " . sprintf("0x%X", $method);
    }
    $debugFileBuffer .= ")";
    createInstructionInfo($action);
}

# Output ActionBranchIfTrue/ActionBranchIfFalse/ActionBranchAlways action as assembly code
sub branchPrintFunction
{
    my $action = shift;
    my $nextAction = shift;
    my $actionName = shift;

    my $branchTo = $action->BranchOffset;

    # BranchOffset[1] is relative offset of branch target instruction to the followed instruction
    my $branchOffset = (split /#/, $branchTo)[1];

    my $originalOffset = $action->OriginalStreamOffset;
    my $targetInstructioinOffset = $nextAction->OriginalStreamOffset + $branchOffset;
    my $finalTargetOffset = $originalToFinalOffsetMap{$targetInstructioinOffset};
    
    $debugFileBuffer .= "$actionName\t$finalTargetOffset";
    
    if ( !defined($debugFile) )
    {
        return;
    }

    # Add next statement information to debug info.
    die "Unkown error occurs when generating branch debug information" if ( !defined($originalToFinalOffsetMap{$originalOffset}) || !defined($originalToFinalOffsetMap{$targetInstructioinOffset}) );

    my $currentStatementInfo = $debugFile->getStatementInfoByOffset($originalToFinalOffsetMap{$originalOffset});
    my $targetStatementInfo = $debugFile->getStatementInfoByOffset($finalTargetOffset);

    if ( defined($currentStatementInfo) )
    {
        my $functionInfo = $currentStatementInfo->getFunction();
        if ( defined($targetStatementInfo) )
        {
            $currentStatementInfo->addNextStatementOffset($finalTargetOffset);
            createInstructionInfo($action, $finalTargetOffset);
        }
        else
        {
            # If target statement information is not found, it should be outside of function.
            $currentStatementInfo->addNextStatementOffset(0xFFFFFFFF);
            createInstructionInfo($action, 0xFFFFFFFF);
        }
    }
    else
    {
        createInstructionInfo($action, $finalTargetOffset);
    }
}

# Output ActionBranchIfTrue action as assembly code
sub branchIfTruePrintFunction
{
    branchPrintFunction(@_, "branchIfTrue");
}

# Output ActionBranchAlways action as assembly code
sub branchAlwaysPrintFunction
{
    branchPrintFunction(@_, "branchAlways");
    
    if ( !defined($debugFile) )
    {
        return;
    }
 
    my $action = shift;
    my $branchTo = $action->BranchOffset;

    # BranchOffset[1] is relative offset of branch target instruction to the followed instruction
    my $branchOffset = (split /#/, $branchTo)[1];

    my $originalOffset = $action->OriginalStreamOffset;
    my $currentStatementInfo = $debugFile->getStatementInfoByOffset($originalToFinalOffsetMap{$originalOffset});
    
    # Save the statement information instance in a hash table. We'll ignore these statements in the link process.
    if ( defined($currentStatementInfo) )
    {
        $alwaysJumpStatementList{$currentStatementInfo} = $originalToFinalOffsetMap{$originalOffset};
    }
    
    # Save the instruction information instance to a hash table. We'll ignore these instructions in the link process.
    $alwaysJumpInstructionList{$instructionsList[-1]} = $originalToFinalOffsetMap{$originalOffset};
}

# Output ActionBranchIfFalse action as assembly code
sub branchIfFalsePrintFunction
{
    branchPrintFunction(@_, "branchIfFalse");
}

# Output ActionGotoFrame action as assembly code
sub gotoFramePrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "gotoFrame " . $action->Frame;
    createInstructionInfo($action);
}

# Output ActionGotoFrame2 action as assembly code
sub gotoFrame2PrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "gotoFrame2 " . $action->PlayFlag;
    createInstructionInfo($action);
}

# Output ActionSetTarget action as assembly code
sub setTargetPrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "actionSetTarget " . $action->TargetName;
    createInstructionInfo($action);
}

# Output ActionGotoLabel action as assembly code
sub gotoLabelPrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "gotoLabel " . $action->Label;
    createInstructionInfo($action);
}

# Output ActionGetUrl action as assembly code
sub getUrlPrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "getUrl '" . $action->UrlString . "', target='" . $action->TargetString . "'";
    createInstructionInfo($action);
}

# Output ActionWith action as assembly code
sub withPrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "with " . $action->CodeSize;
    createInstructionInfo($action);
}

# Output ActionDefineFunction action as assembly code
sub defineFunctionPrintFunction
{
    my $action = shift;
    my $nextAction = shift;

    my @params = @{$action->Params};
    my $paramCount = $#params;
    $debugFileBuffer .= "function " . ($action->FunctionName->value eq "" ? "[unnamed]" : $action->FunctionName->value) . " (";
    my $i = 0;
    foreach my $param (@params)
    {
        $debugFileBuffer .= $param->value;
        if($i < $paramCount)
        {
            $debugFileBuffer .= ", ";
        }
        $i++;
    }
    $debugFileBuffer .= ")";
    
    if ( !defined($debugFile) )
    {
        return;
    }
    
    # $action->CodeSize[1] is relative offset of next instruction to the followed instruction
    my $nextInstructionOffset = (split /#/, $action->CodeSize)[1];
    
    $nextInstructionOffset = $nextAction->OriginalStreamOffset + $nextInstructionOffset;
    my $finalNextInstructionOffset = $originalToFinalOffsetMap{$nextInstructionOffset};
    
    if ( !defined($finalNextInstructionOffset) )
    {
        $finalNextInstructionOffset = 0xFFFFFFFF;
    }
    
    # AptActionDefineFunction only define function, not executing instructions inside the function.
    # So PC is always move to the instruction after the last instruction of function .
    createInstructionInfo($action, $finalNextInstructionOffset);
    
    # Only save instruction in %alwaysJumpInstructionList, because AptActionDefineFunction is not belongs to any statement.
    $alwaysJumpInstructionList{$instructionsList[-1]} = $finalNextInstructionOffset;
}

# Output ActionStoreRegister action as assembly code
sub storeRegisterPrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "storeRegister r:" . $action->Register;
    createInstructionInfo($action);
}

# Output ActionTry action as assembly code
sub tryPrintFunction
{
    my $action = shift;

    $debugFileBuffer .= "try ";
    $debugFileBuffer .= "[ trySize = " .       $action->TrySize;
    $debugFileBuffer .= ", CatchSize = " .     $action->CatchSize;
    $debugFileBuffer .= ", FinallySize = " .   $action->FinallySize;
    $debugFileBuffer .= ", CatchName = " .     $action->CatchName;
    $debugFileBuffer .= ", CatchRegister = " . $action->CatchRegister;
    $debugFileBuffer .= " ]";
    createInstructionInfo($action);
}

# Output ActionDefineFunction2 action as assembly code
sub defineFunction2PrintFunction
{
    my $action = shift;
    my $nextAction = shift;

    my @params = @{$action->Parameters};
    my $paramCount = $#params;
    $debugFileBuffer .= "function2 " . ($action->FunctionName->value eq "" ? "[unnamed]" : $action->FunctionName->value) . " (";
    my $i = 0;
    foreach my $param (@params)
    {
        $debugFileBuffer .= $param->ParamName;
        if($i < $paramCount)
        {
            $debugFileBuffer .= ", ";
        }
        $i++;
    }
    $debugFileBuffer .= ")";
    
    # $action->CodeSize[1] is relative offset of next instruction to the followed instruction
    my $nextInstructionOffset = (split /#/, $action->CodeSize)[1];
    
    $nextInstructionOffset = $nextAction->OriginalStreamOffset + $nextInstructionOffset;
    my $finalNextInstructionOffset = $originalToFinalOffsetMap{$nextInstructionOffset};
    
    if ( !defined($finalNextInstructionOffset) )
    {
        $finalNextInstructionOffset = 0xFFFFFFFF;
    }
    
    # AptActionDefineFunction2 only define function, not executing instructions inside the function.
    # So PC is always move to the instruction after the last instruction of function .
    createInstructionInfo($action, $finalNextInstructionOffset);
    
    # Only save instruction in %alwaysJumpInstructionList, because AptActionDefineFunction2 is not belongs to any statement.
    $alwaysJumpInstructionList{$instructionsList[-1]} = $finalNextInstructionOffset;
}

# Output debug information for actions which quit current function.
sub quitFunctionPrintFunction
{
    my $action = shift;
    
    my $tag = $action->Tag->value;

    $debugFileBuffer .= $simpleActionPrintText{$tag};
    
    if ( !defined($debugFile) )
    {
        return;
    }
    
    my $originalOffset = $action->OriginalStreamOffset;
    my $currentStatementInfo = $debugFile->getStatementInfoByOffset($originalToFinalOffsetMap{$originalOffset});
    
    if ( defined($currentStatementInfo) )
    {
        $currentStatementInfo->addNextStatementOffset(0xFFFFFFFF);
    }
    
    createInstructionInfo($action, 0xFFFFFFFF);
}

# Output AptActionEnd action as assembly code
sub endActionPrintFunction
{
    my $action = shift;
    my $tag = $action->Tag->value;
    
    $debugFileBuffer .= $simpleActionPrintText{$tag};
    
    createInstructionInfo($action, 0xFFFFFFFF);
}

# Mapping from action code to debug output function.
my %actionPrintHash =
(
    0x00 => \&endActionPrintFunction,
    0x04 => \&simpleActionPrintFunction,
    0x05 => \&simpleActionPrintFunction,
    0x06 => \&simpleActionPrintFunction,
    0x07 => \&simpleActionPrintFunction,
    0x08 => \&simpleActionPrintFunction,
    0x09 => \&simpleActionPrintFunction,
    0x0A => \&simpleActionPrintFunction,
    0x0B => \&simpleActionPrintFunction,
    0x0C => \&simpleActionPrintFunction,
    0x0D => \&simpleActionPrintFunction,
    0x0E => \&simpleActionPrintFunction,
    0x0F => \&simpleActionPrintFunction,
    0x10 => \&simpleActionPrintFunction,
    0x11 => \&simpleActionPrintFunction,
    0x12 => \&simpleActionPrintFunction,
    0x13 => \&simpleActionPrintFunction,
    0x14 => \&simpleActionPrintFunction,
    0x15 => \&simpleActionPrintFunction,
    0x17 => \&simpleActionPrintFunction,
    0x18 => \&simpleActionPrintFunction,
    0x1C => \&simpleActionPrintFunction,
    0x1D => \&simpleActionPrintFunction,
    0x20 => \&simpleActionPrintFunction,
    0x21 => \&simpleActionPrintFunction,
    0x22 => \&simpleActionPrintFunction,
    0x23 => \&simpleActionPrintFunction,
    0x24 => \&simpleActionPrintFunction,
    0x25 => \&simpleActionPrintFunction,
    0x26 => \&simpleActionPrintFunction,
    0x27 => \&simpleActionPrintFunction,
    0x28 => \&simpleActionPrintFunction,
    0x29 => \&simpleActionPrintFunction,
    0x2A => \&simpleActionPrintFunction,
    0x2B => \&simpleActionPrintFunction,
    0x2C => \&simpleActionPrintFunction,
    0x30 => \&simpleActionPrintFunction,
    0x31 => \&simpleActionPrintFunction,
    0x32 => \&simpleActionPrintFunction,
    0x33 => \&simpleActionPrintFunction,
    0x34 => \&simpleActionPrintFunction,
    0x35 => \&simpleActionPrintFunction,
    0x36 => \&simpleActionPrintFunction,
    0x37 => \&simpleActionPrintFunction,
    0x3A => \&simpleActionPrintFunction,
    0x3B => \&simpleActionPrintFunction,
    0x3C => \&simpleActionPrintFunction,
    0x3D => \&simpleActionPrintFunction,
    0x3E => \&quitFunctionPrintFunction,
    0x3F => \&simpleActionPrintFunction,
    0x40 => \&simpleActionPrintFunction,
    0x41 => \&simpleActionPrintFunction,
    0x42 => \&simpleActionPrintFunction,
    0x43 => \&simpleActionPrintFunction,
    0x44 => \&simpleActionPrintFunction,
    0x45 => \&simpleActionPrintFunction,
    0x46 => \&simpleActionPrintFunction,
    0x47 => \&simpleActionPrintFunction,
    0x48 => \&simpleActionPrintFunction,
    0x49 => \&simpleActionPrintFunction,
    0x4A => \&simpleActionPrintFunction,
    0x4B => \&simpleActionPrintFunction,
    0x4C => \&simpleActionPrintFunction,
    0x4D => \&simpleActionPrintFunction,
    0x4E => \&simpleActionPrintFunction,
    0x4F => \&simpleActionPrintFunction,
    0x50 => \&simpleActionPrintFunction,
    0x51 => \&simpleActionPrintFunction,
    0x52 => \&simpleActionPrintFunction,
    0x53 => \&simpleActionPrintFunction,
    0x54 => \&simpleActionPrintFunction,
    0x55 => \&simpleActionPrintFunction,
    0x56 => \&simpleActionPrintFunction,
    0x58 => \&simpleActionPrintFunction,
    0x59 => \&simpleActionPrintFunction,
    0x5a => \&simpleActionPrintFunction,
    0x5b => \&simpleActionPrintFunction,
    0x5c => \&simpleActionPrintFunction,
    0x5d => \&simpleActionPrintFunction,
    0x5e => \&simpleActionPrintFunction,
    0x60 => \&simpleActionPrintFunction,
    0x61 => \&simpleActionPrintFunction,
    0x62 => \&simpleActionPrintFunction,
    0x63 => \&simpleActionPrintFunction,
    0x64 => \&simpleActionPrintFunction,
    0x65 => \&simpleActionPrintFunction,
    0x66 => \&simpleActionPrintFunction,
    0x67 => \&simpleActionPrintFunction,
    0x69 => \&simpleActionPrintFunction,
    0x70 => \&simpleActionPrintFunction,
    0x71 => \&simpleActionPrintFunction,
    0x72 => \&simpleActionPrintFunction,
    0x73 => \&simpleActionPrintFunction,
    0x74 => \&simpleActionPrintFunction,
    0x75 => \&simpleActionPrintFunction,
    0x76 => \&simpleActionPrintFunction,
    0x77 => \&simpleActionPrintFunction,
    0x81 => \&gotoFramePrintFunction,
    0x83 => \&getUrlPrintFunction,
    0x87 => \&storeRegisterPrintFunction,
    0x88 => \&constantPoolPrintFunction,
    0x8a => \&simpleActionPrintFunction,
    0x8b => \&setTargetPrintFunction,
    0x8c => \&gotoLabelPrintFunction,
    0x8e => \&defineFunction2PrintFunction,
    0x8F => \&tryPrintFunction,
    0x94 => \&withPrintFunction,
    0x96 => \&pushPrintFunction,
    0x99 => \&branchAlwaysPrintFunction,
    0x9a => \&getUrl2PrintFunction,
    0x9b => \&defineFunctionPrintFunction,
    0x9d => \&branchIfTruePrintFunction,
    0x9e => \&simpleActionPrintFunction,
    0x9f => \&gotoFrame2PrintFunction,
    0xa1 => \&simpleActionPrintFunction,
    0xa2 => \&simpleActionPrintFunction,
    0xa3 => \&simpleActionPrintFunction,
    0xa4 => \&simpleActionPrintFunction,
    0xa5 => \&simpleActionPrintFunction,
    0xa6 => \&simpleActionPrintFunction,
    0xa7 => \&simpleActionPrintFunction,
    0xae => \&simpleActionPrintFunction,
    0xaf => \&simpleActionPrintFunction,
    0xb0 => \&simpleActionPrintFunction,
    0xb1 => \&simpleActionPrintFunction,
    0xb2 => \&simpleActionPrintFunction,
    0xb3 => \&simpleActionPrintFunction,
    0xb4 => \&simpleActionPrintFunction,
    0xb5 => \&simpleActionPrintFunction,
    0xb6 => \&simpleActionPrintFunction,
    0xb7 => \&simpleActionPrintFunction,
    0xb8 => \&branchIfFalsePrintFunction,
    0xb9 => \&simpleActionPrintFunction,
);

# Record action offset mapping
sub addActionOffsetMapping($$)
{
    my $action = shift;
    my $finalOffset = shift;

    if (defined $fileOffsetFromAction{$action})
    {
        die "Offset map already has stuff for action: " . $action;
    }

    $fileOffsetFromAction{$action} = $finalOffset;
    push @offsetList, { original => scalar($action->OriginalStreamOffset()), final => $finalOffset };
}

# Record control offset mapping
sub addControlOffsetMapping($$)
{
    my $control = shift;
    my $finalOffset = shift;

    if (defined $fileOffsetFromControl{$control})
    {
        die "Offset map already has stuff for control: " . $control;
    }

    $fileOffsetFromControl{$control} = $finalOffset;
}

# Record character offset mapping
sub addCharacterOffsetMapping($$)
{
    my $character = shift;
    my $finalOffset = shift;

    if (defined $fileOffsetFromCharacter{$character})
    {
        die "Offset map already has stuff for character: " . $character;
    }            
    $fileOffsetFromCharacter{$character} = $finalOffset;
}

# Write debugging information about a set of actions
# This function mirrors the passes in writeactions and writeactionsbody.
sub writeDebugActions
{
    my $actions = shift;
    for (my $i = 0; $i <= $#$actions; $i++)
    {
        my $action = $actions->[$i];
        my $tag = $action->Tag->value;

        # Start off action lines with the file offset in the .apt file where this action can be found
        # $debugFileBuffer .= $action->OriginalStreamOffset . "\tAction\t" . $fileOffsetFromAction{$action} . "\t" . "\t";
        $debugFileBuffer .= "Action\t" . $fileOffsetFromAction{$action} . "\t" . "\t";

        die "Unexpected action code is met!" if (!defined($actionPrintHash{$tag}));

        my $nextAction = undef;
        if ($i < $#$actions )
        {
            $nextAction = $actions->[$i + 1];
        }
        $actionPrintHash{$tag}->($action, $nextAction);

        $debugFileBuffer .= "\n";
        $currentAssemblyLine++;
    }
}

# Write debugging information about the frames of a Movie
# Params:
#   The movie
sub writeDebugMovieFrames
{
    # Loop through all frames of this movie to get all controls.
    # Delve into controls that have juicy debugging info.
    my $movie = shift;
    my $nFrames = ($#{$movie} + 1);
    for (my $i = 0; $i < $nFrames; $i++)
    {
        my $nControls = ($#{$movie->[$i]} + 1);
        
        
        for (my $j = 0; $j < $nControls; $j++)
        {
            my $control = $movie->[$i][$j];
            $debugFileBuffer .= "Control\t" . $fileOffsetFromControl{$control} . "\t";
        
            # TODO Some of the below could theoretically use more debugging information
            # if we felt like providing it. You can glean more information about exactly
            # what is stored by looking at the writemovieframes function
            if ($control->{type} eq 'PlaceObject2' || $control->{type} eq 'PlaceObject3')
            {
                if ($control->{type} eq 'PlaceObject2')
                {
                    $debugFileBuffer .= "PlaceObject2\n";
                }
                else
                {
                    $debugFileBuffer .= "PlaceObject3\n";
                }
                $currentAssemblyLine++;
                
                if (defined $control->{actions})
                {
                    foreach my $action (@{$control->{actions}})
                    {
                        writeDebugActions $action->{actions};
                    }
                }
            }
            elsif ($control->{type} eq 'SetBackgroundColor')
            {
                $debugFileBuffer .= "SetBackgroundColor\n";
                $currentAssemblyLine++;
            }
            elsif ($control->{type} eq 'DoAction')
            {
                $debugFileBuffer .= "DoAction\n";
                $currentAssemblyLine++;
                writeDebugActions $control->{actions};
            }
            elsif ($control->{type} eq 'DoInitAction')
            {
                $debugFileBuffer .= "DoInitAction\n";
                $currentAssemblyLine++;
                writeDebugActions $control->{actions};
            }
            elsif ($control->{type} eq 'RemoveObject2')
            {
                $debugFileBuffer .= "RemoveObject2\n";
                $currentAssemblyLine++;
            }
            elsif ($control->{type} eq 'FrameLabel')
            {
                $debugFileBuffer .= "FrameLabel\n";
                $currentAssemblyLine++;
            }
            elsif ($control->{type} eq 'StartSound')
            {
                $debugFileBuffer .= "StartSound\n";
                $currentAssemblyLine++;
            }
            elsif ($control->{type} eq 'SoundStreamHead')
            {
                $debugFileBuffer .= "SoundStreamHead\n";
                $currentAssemblyLine++;
            }
            else
            {
                $debugFileBuffer .= "Unknown control " . $control->{type} . "\n";
                $currentAssemblyLine++;
            }
        }
    }
}

# Load pre-build debug information from file.
sub loadDebugInfo($)
{
    my $inputFileName = shift;

    my $file;
    open($file, $inputFileName . ".apd") || die "Can't open " . $inputFileName . ".apd: $!";
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
    $debugFile = DebugFile->new();
    $debugFile->load($stream);
}

# Save debug information to file
sub saveDebugInfo($)
{
    if ( !defined($debugFile) )
    {
        return;
    }

    my $outputFileName = shift;
    
    my $file;
    open($file, "> $outputFileName.apd") or die "Can't open $outputFileName.apd: $!";
    binmode($file);
    my $stream = SWF::BinStream::Write->new();
    #$stream->autoflush(1000, sub {print $file $_[1]});
    $debugFile->save($stream);
    print $file $stream->flush_stream;
    close $file;
}

# Adjust instruction offsets for source code level
sub remapSourceCodeDebugInfo()
{
    my $maxOffsetIndex = (scalar(@offsetList) - 1);

    # Sort offset list by original offset
    @offsetList = sort {$a->{'original'} <=> $b->{'original'}} @offsetList;

    for my $i (0..$maxOffsetIndex)
    {
        die "Duplicated original offset is found!" if (defined($originalToFinalOffsetMap{$offsetList[$i]{'original'}}));

        $originalToFinalOffsetMap{$offsetList[$i]{'original'}} = $offsetList[$i]{'final'};
    }
    
    if ( !defined($debugFile) )
    {
        return;
    }

    # First pass, process the end of function.
    for my $internalFile (@{$debugFile->getFileList()})
    {
        for my $functionInfo (@{$internalFile->getFunctionInfoList()})
        {
            for my $statementInfo (@{$functionInfo->getStatementInfoList()})
            {
                # Get statement range first (begin index and end index)
                my $beginOffset = $statementInfo->getBeginOffset();
                my $endOffset = $statementInfo->getEndOffset();
                my $beginIndex = searchOffsets($beginOffset);
                my $endIndex = searchOffsets($endOffset);

                # If the statement doesn't have any debugging info assocaited with it, we'll just skip it.
                # This happens sometimes - Flash seems to output debug files with missing data now and again.
                # A fix that apears to work pretty well is to clear ASO files out and republish.  But we don't
                # care to have the pipeline break when we hit this situation, so we'll just ignore it
                if ( -1 != $beginIndex && -1 != $endIndex )
                {
                    my $minOffset = 0xFFFFFFFF;
                    my $maxOffset = 0;

                    for my $i ($beginIndex..$endIndex)
                    {
                        if ( $minOffset > $offsetList[$i]{'final'} )
                        {
                            $minOffset = $offsetList[$i]{'final'};
                        }
                        if ( $maxOffset < $offsetList[$i]{'final'} )
                        {
                            $maxOffset = $offsetList[$i]{'final'};
                        }
                    }

                    die "Error occurs when remapping debug information offset: function end offset is less than begin offset" if ( $maxOffset < $minOffset );

                    $statementInfo->setBeginOffset($minOffset);
                    $statementInfo->setEndOffset($maxOffset);
                }
                else
                {
                    # Is there some way to remove this statement or something?
                }
            }
            
            # The last statement must be the end of function, next statement index is -1 for quit function.
            if ( @{$functionInfo->getStatementInfoList()} > 0 )
            {
                @{$functionInfo->getStatementInfoList()}[-1]->addNextStatementOffset(0xFFFFFFFF);
            }
        }
    }

    $debugFile->updateOffsets();
}

# Search offsetList for a given original offset
# The list should already be sorted by original offset before calling this
# Returns the index containing the offset or -1 if not found
sub searchOffsets($)
{
    my $find = shift;

    # Do a binary search for the offset
    my ($lower, $upper) = (0, $#offsetList);
    my $i;
    while ($lower <= $upper)
    {
        $i = int(($lower + $upper) / 2);
        if ($offsetList[$i]{'original'} < $find)
        {
            $lower = $i + 1; 
        }
        elsif ($offsetList[$i]{'original'} > $find)
        {
            $upper = $i - 1;
        }
        else
        {
            return $i;
        }
    }
    return -1;
}

# Link source code statements. (Add next potential statment offset to each statements if possible)
sub linkSourceCodeDebugInfo()
{
    if ( !defined($debugFile) )
    {
        return;
    }
    # Second pass, process all statements which are not end of function ( return )
    for my $internalFile (@{$debugFile->getFileList()})
    {
        for my $functionInfo (@{$internalFile->getFunctionInfoList()})
        {
            if ( @{$functionInfo->getStatementInfoList()} > 0 )
            {
                my $maxIndex = @{$functionInfo->getStatementInfoList()} - 1;
                for my $i (0..$maxIndex)
                {
                    my $statementInfo = @{$functionInfo->getStatementInfoList()}[$i];
                    if ( !defined($alwaysJumpStatementList{$statementInfo}) )
                    {
                        my $isEndOfFunction = 0;
                        for my $offset (@{$statementInfo->getNextStatementOffsetList()})
                        {
                            if ( 0xFFFFFFFF == $offset )
                            {
                                $isEndOfFunction = 1;
                                last;
                            }
                        }
                        
                        if ( 0 == $isEndOfFunction )
                        {
                            die "Unknow error occurs! Last statement of function is not set flag of end!" if ( $maxIndex == $i );
                            my $nextStatementInfo = @{$functionInfo->getStatementInfoList()}[$i + 1];
                            $statementInfo->addNextStatementOffset($nextStatementInfo->getBeginOffset());
                        }
                    }
                }
            }
        }
    }
}

# Adjust instruction offsets for assembly code level
sub remapAssemblyDebugInfo()
{
    if ( !defined($debugFile) )
    {
        return;
    }
    
    $debugFile->getAssemblyFile()->setAssemblyCode($debugFileBuffer);
    
    # First pass, process the end of function.
    for my $internalFile (@{$debugFile->getFileList()})
    {
        for my $functionInfo (@{$internalFile->getFunctionInfoList()})
        {
            if ( @{$functionInfo->getStatementInfoList()} > 0 )
            {
                # Function end offset if inclusive, which means it's the last instruction of the function.
                my $endOffset = @{$functionInfo->getStatementInfoList()}[-1]->getEndOffset();
                my $instructionInfo = $debugFile->getInstructionByOffset($endOffset);
                
                # Some instructions may not actually be in the debug file, in which case we will get back an
                # undefined valid here which we can skip
                if (defined($instructionInfo))
                {
                    $instructionInfo->addNextInstructionByOffset(0xFFFFFFFF);
                }
            }
        }
    }
}

# Link assembly instructions. (Add next potential instruction offset to each instructions if possible)
sub linkAssemblyDebugInfo()
{
    if ( !defined($debugFile) )
    {
        return;
    }
    
    # Second pass, process all instructions which are not end of function ( return )
    my $instructionList = $debugFile->getAssemblyFile()->getInstructionList();
    my $maxInstructionIndex = @{$instructionList} - 1;
    for my $i (0..$maxInstructionIndex)
    {
        my $instructionInfo = @{$instructionList}[$i];
        if ( !defined($alwaysJumpInstructionList{$instructionInfo}) )
        {
            my $isEnd = 0;
            for my $offset (@{$instructionInfo->getNextInstructinOffsetList()})
            {
                if ( 0xFFFFFFFF == $offset )
                {
                    $isEnd = 1;
                    last;
                }
            }
            
            if ( 0 == $isEnd )
            {
                die "Unknow error occurs! Last instruction of file is not set flag of end!" if ( $maxInstructionIndex == $i );
                my $nextInstructionInfo = @{$instructionList}[$i + 1];
                $instructionInfo->addNextInstructionByOffset($nextInstructionInfo->getOffset());
            }
        }
    }
}

# Disassemble bytecode stream
sub disassemble($)
{
    my $characters = shift;
    my $nCharacters = scalar @$characters;
    
    for (my $charID = 0; $charID < $nCharacters; $charID++)
    {
        my $character = $characters->[$charID];

        unless (defined $character)
        {
            next;
        }
        
        # TODO Some of the below could theoretically use more debugging information
        # if we felt like providing it. You can glean more information about exactly
        # what is stored by looking at the toFile function
        $debugFileBuffer .= "Character\t" . $fileOffsetFromCharacter{$character} . "\t";

        if ($character->{type} eq 'shape')
        {
            $debugFileBuffer .= "AptCharacterType_Shape\n";
            $currentAssemblyLine++;
        }
        elsif ($character->{type} eq 'morph')
        {
            $debugFileBuffer .= "AptCharacterType_Morph\n";
            $currentAssemblyLine++;
        }
        elsif ($character->{type} eq 'sprite')
        {
            $debugFileBuffer .= "AptCharacterType_Sprite\n";
            $currentAssemblyLine++;
            writeDebugMovieFrames $character->{movie};
        }
        elsif ($character->{type} eq 'animation')
        {
            $debugFileBuffer .= "AptCharacterType_Animation\n";
            $currentAssemblyLine++;
            writeDebugMovieFrames $character->{movie};
        }
        elsif ($character->{type} eq 'sound')
        {
            $debugFileBuffer .= "AptCharacterType_Sound\n";
            $currentAssemblyLine++;
        }
        elsif ($character->{type} eq 'texture')
        {
            $debugFileBuffer .= "AptCharacterType_Bitmap\n";
            $currentAssemblyLine++;
        }
        elsif ($character->{type} eq 'button')
        {
            $debugFileBuffer .= "AptCharacterType_Button\n";
            $currentAssemblyLine++;
            foreach my $ar (@{$character->{actionrecords}})
            {
                writeDebugActions $ar->{actions};
            }
        }
        elsif ($character->{type} eq 'font')
        {
            $debugFileBuffer .= "AptCharacterType_Font\n";
            $currentAssemblyLine++;
        }
        elsif ($character->{type} eq 'text')
        {
            $debugFileBuffer .= "AptCharacterType_Text\n";
            $currentAssemblyLine++;
        }
        elsif ($character->{type} eq 'statictext')
        {
            $debugFileBuffer .= "AptCharacterType_StaticText\n";
            $currentAssemblyLine++;
        }
        elsif ($character->{type} eq 'video')
        {
            $debugFileBuffer .= "AptCharacterType_Video\n";
            $currentAssemblyLine++;
        }
        else
        {
            $debugFileBuffer .= "Unknown character " . $character->{type} . "\n";
            $currentAssemblyLine++;
        }
    }
}

# If we're outputting a debug file, we will want to go through another pass of
# all the data and write out debugging information.
# Params:
#   Name of the file
#   Characters in the file
sub generateDebugFile
{
    my $filename = shift;
    my $characters = shift;

    $filename =~ s/\.eaf$//i;

    # Load debug information first
    loadDebugInfo($filename);

    # All instructions offset are already recorded, so we update source code level offset here.
    remapSourceCodeDebugInfo();

    # Disassemble bytecode stream
    disassemble($characters);

    # Link source code statement information
    linkSourceCodeDebugInfo();

    remapAssemblyDebugInfo();
    linkAssemblyDebugInfo();

    saveDebugInfo($filename);

    open OUT, ">$filename.debug" or die "couldn't open '$filename.debug' for output";
    syswrite OUT, $debugFileBuffer;
    close OUT;
}

1;