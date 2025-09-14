# ***********************************************************************************************
#
#
#   \file    DefineShape.pl
#
#   \brief   SWF file decompiler
#
#   (c) 2007 Electronic Arts Inc.
#
#   \version    2.06.00    11/15/07 (RB) Last Version to support only SWF 7
#   \version    2.07.00    04/29/08 (RB) Handles DefineShape4. Handles focal radial gradient but
#                                        just passes through to radial gradient.
#
# ***********************************************************************************************

package AptDerive;
use strict;
use warnings;
use Storable;
use AptMessage ':all';
use AptImage;

my $PI = 3.1415926535;

sub _isLeftFill
{
	my ($v0, $v1, $v2) = @_;
	my $dx1 = $v1->[0] - $v0->[0];
	my $dy1 = $v1->[1] - $v0->[1];
	my $dx2 = $v2->[0] - $v1->[0];
	my $dy2 = $v2->[1] - $v1->[1];
	my $angle1 = atan2($dy1, $dx1);
	my $angle2 = atan2($dy2, $dx2);
	my $dangle = $angle1 - $angle2;
	$dangle -= 2*$PI if ($dangle > $PI);
	$dangle += 2*$PI if ($dangle < -$PI);
	return 1 if ($dangle > 0);
	return 0;
}

sub _findEarliestPoint
{
	my ($x, $y, $points) = @_;

	my $lookup = "${x}_${y}";
	if (defined $points->{$lookup})
	{
		return $points->{$lookup};
	}
	my $newval = [ $x, $y ];
	$points->{$lookup} = $newval;
	return $newval;
}

# lame worldspace subdivision to fixed number of steps
sub _buildBezierPoints
{
	my ($nSubdivisions, $edgeList, $points, $a1, $control, $a2, $line, $fill0, $fill1) = @_;

	if ($nSubdivisions != 0)
	{
		my ($nMinX, $nMaxX, $nMinY, $nMaxY);

		if ($a1->[0] < $control->[0])
		{
			$nMinX = $a1->[0];
			$nMaxX = $control->[0];
		}
		else
		{
			$nMinX = $control->[0];
			$nMaxX = $a1->[0];
		}
		$nMinX = $a2->[0] if ($a2->[0] < $nMinX);
		$nMaxX = $a2->[0] if ($a2->[0] > $nMaxX);

		if ($a1->[1] < $control->[1])
		{
			$nMinY = $a1->[1];
			$nMaxY = $control->[1];
		}
		else
		{
			$nMinY = $control->[1];
			$nMaxY = $a1->[1];
		}
	 	$nMinY = $a2->[1] if ($a2->[1] < $nMinY);
		$nMaxY = $a2->[1] if ($a2->[1] > $nMaxY);

		# if less than 1.5 worldspace pixels, don't bother subdividing
		# you must be extremely careful if you change this threshold.
		# sometimes, a straight line from the first anchor to the control point to the second anchor would result in a line
		# segment that overlaps another, whereas the exact bezier curve would not overlap. if this threshold is made larger, the
		# approximation will be insufficiently accurate, overlapping line segments will result, and thus a bad triangulation will
		# occur. this obviously is not a very nice fix. the alternative is to have another process that finds overlapping line
		# segments and somehow "fixes" them. it's not easy or obvious how this process would work (at least to me).
		if (($nMaxX - $nMinX) > 30 && ($nMaxY - $nMinY) > 30)
		{
			my $c1 = _findEarliestPoint(($a1->[0] + $control->[0]) / 2, ($a1->[1] + $control->[1]) / 2, $points);
			my $c2 = _findEarliestPoint(($a2->[0] + $control->[0]) / 2, ($a2->[1] + $control->[1]) / 2, $points);
			my $p = _findEarliestPoint(($c1->[0] + $c2->[0]) / 2, ($c1->[1] + $c2->[1]) / 2, $points);

			$nSubdivisions--;

			_buildBezierPoints($nSubdivisions, $edgeList, $points, $a1, $c1, $p, $line, $fill0, $fill1);
			_buildBezierPoints($nSubdivisions, $edgeList, $points, $p, $c2, $a2, $line, $fill0, $fill1);
			return;
		}
	}

	push @$edgeList, {
		from => $a2,
		to => $control,
		line => $line,
		fill0 => $fill0,
		fill1 => $fill1
	};
	push @$edgeList, {
		from => $control,
		to => $a1,
		line => $line,
		fill0 => $fill0,
		fill1 => $fill1
	};
}


# if there's an edge attached to the triangle, it will do. attached edges are "original" edges from the flash data.
# we can choose any edge, because all of them will have consistent fill specs.
# if there isn't an edge attached to the triangle, we use the neighbour data to move to an arbitrary triangle in any direction
# because if there wasn't an edge on this triangle, we made triangle during triangulation so we just cut a fill in half, so on
# both sides, the fill is the same
# ...
# (at least that's the theory)
sub _findEdgeForTriangle
{
	my ($tri, $triList) = @_;
	my @visitedTri;
again:
	if ($#{$tri->{edgedata}} >= 0)
	{
		return $tri->{edgedata}[0];
	}

	for (my $i = 0; $i < 3; $i++)
	{
		my $neighbour = $tri->{neighbours}[$i];
		if ($neighbour == -1)
		{
			return undef;
		}

		next if ($visitedTri[$neighbour]);
		$visitedTri[$neighbour] = 1;
		$tri = $triList->[$neighbour];
		goto again;
	}
	return undef;
}

sub DefineShape_triangulate
{
	my ($shape, $edges) = @_;

	my %pointHash;
	my @points;
	my @potentialTriangles;
	my $verbose;
	$verbose = 0;

	# build mapping from points to their index in the points array
	# edges->[$i]{from}/{to} holds (shared) refs to 2 sized arrays that are points.
	# as new points are encountered, add them to @points, and maintain a hash that
	# maps from the array reference that is the point to the index of the point.
	for (my $i = 0; $i <= $#$edges; $i++)
	{
		if (!defined $pointHash{$edges->[$i]{from}})
		{
			push @points, $edges->[$i]{from};
			$pointHash{$edges->[$i]{from}} = $#points;
		}
		if (!defined $pointHash{$edges->[$i]{to}})
		{
			push @points, $edges->[$i]{to};
			$pointHash{$edges->[$i]{to}} = $#points;
		}
	}

	# spit out the lines and points that make up all the edges we have into a format
	# that triangle.exe can read, and then get it to do generate a triangulation.
	# it's important that the convex hull switch is on so that we don't lose tris.
	print "triangulating...\n" if $verbose;

	my $filetempbase;
	my $filetempnum;
	my $filetempname;
	$filetempbase = "swfctmp";
	$filetempnum = int(rand(500000));
	while((-e $filetempbase.$filetempnum.".poly") || (-e $filetempbase.$filetempnum.".1.node") || (-e $filetempbase.$filetempnum.".1.ele") || (-e $filetempbase.$filetempnum.".1.neigh"))
	{
		$filetempnum = int(rand(500000));
	}
	$filetempname = $filetempbase.$filetempnum;

	open OUT, ">".$filetempname.".poly" or die "couldn't open '".$filetempname.".poly' for write";

	my $numPoints = $#points + 1;

	if ($numPoints < 3)
	{
		message 1, "warning: only $numPoints points in shape.\n";
		return;
	}

	print OUT "$numPoints 2 0 0\n";
	for (my $i = 0; $i < $numPoints; $i++)
	{
		print OUT "$i $points[$i][0] $points[$i][1]\n";
	}
	my $numEdges = $#$edges + 1;
	print OUT "$numEdges 0\n";
	for (my $i = 0; $i < $numEdges; $i++)
	{
		my $f = $pointHash{$edges->[$i]{from}};
		my $t = $pointHash{$edges->[$i]{to}};
		print OUT "$i $f $t\n";
	}
	print OUT "0\n";
	close OUT;

	system("triangle -Q -P -c -n -p ".$filetempname.".poly");
	die $@ if ($@);

=pod
	open OUT, ">".$filetempname.".txt" or die "couldn't open '".$filetempname.".txt' for write";
	print OUT "c\n";
	print OUT "s l:1:0:0:0:255\n";
	for (my $i = 0; $i < $numEdges; $i++)
	{
		my $f = $edges->[$i]{from};
		my $t = $edges->[$i]{to};

		print OUT "l ", $f->[0], ":", $f->[1], ":", $t->[0], ":", $t->[1], "\n";
	}
	close OUT;
=cut


	# read back in the data that triangle generated.
	# .node is all the points (list of all the points we spit out, plus new ones)
	# .ele is the triangle list; indices into the .node 
	# .neigh is neighbour information for each triangle. the first neighbour of triangle i is opposite the first
	# corner of triangle i. -1 indicates no neighbour.
	open NODE, $filetempname.".1.node" or die "couldn't node read input from triangle";
	open ELE, $filetempname.".1.ele" or die "couldn't ele read input from triangle";
	open NEIGH, $filetempname.".1.neigh" or die "couldn't neigh read input from triangle";

	my $nodeHeader = <NODE>;
	my $eleHeader = <ELE>;
	my $neighHeader = <NEIGH>;
	my @nodeHeaderData = split /\s+/, $nodeHeader;
	my @eleHeaderData = split /\s+/, $eleHeader;
	my @neighHeaderData = split /\s+/, $neighHeader;

	if ($verbose >= 2)
	{
		print "nodeHeaderData:\n";
		print Dumper(\@nodeHeaderData);
		print "eleHeaderData:\n";
		print Dumper(\@eleHeaderData);
		print "neighHeaderData:\n";
		print Dumper(\@neighHeaderData);
	}

	if ($nodeHeaderData[1] ne '2' || $eleHeaderData[2] ne '0') { die "don't understand node header data from triangle"; }
	if ($eleHeaderData[1] ne '3' || $eleHeaderData[2] ne '0') { die "don't understand ele header data from triangle"; }
	if ($neighHeaderData[1] ne '3') { die "don't understand neigh header data from triangle"; }

	my $numVertices = $nodeHeaderData[0];
	print "points = $numPoints\n" if $verbose;
	print "numVertices = $numVertices\n" if $verbose;

	# if there's line segment intersections, triangle will generate new points at the intersections and correctly triangulate.
	# flash won't actually generate overlapping shapes, so you'd think this wouldn't happen. however, because we approximate
	# bezier curves using triangles, it's possible that the linear approximation to the curve will intersect, whereas the exact
	# curve would not intersect. afai can tell, that's the only time that new vertices will be generated. in this case, we read in
	# the new points and add them to the point list.
	# TODO: probably need to update some other stuff here? 
	if ($numPoints != $numVertices)
	{
		# XXX triangle is now hacked to not generate not verts (and thus allow overlapping tris); this should never happen now
		#die "internal error: triangulation generated new vertices.\n";
#=pod
		print "\torig = $numPoints\n";
		print "\tnew = $numVertices\n";

		# read in any new points that were generated.
		for (my $i = 0; $i < $numVertices; $i++)
		{
			my $nodeLine = <NODE>;
			$nodeLine =~ s/^\s*//;
			my @coords = split /\s+/, $nodeLine;
			if ($i >= $numPoints)
			{
				push @points, [bless(do{\($coords[1])}, 'SWF::Element::Scalar'), bless(do{\($coords[2])}, 'SWF::Element::Scalar')];
			}
		}
#=cut

	}

	my $numTris = $eleHeaderData[0];
	die "neigh and ele don't match" if $numTris != $neighHeaderData[0];

	# read in all the triangles and their neighbour information from the node and neigh files.
	# this gives us a list of all the possible triangles that we could end up drawing.
	print "numTris = $numTris\n" if $verbose;
	for (my $i = 0; $i < $numTris; $i++)
	{
		my $eleLine = <ELE>;
		my $neighLine = <NEIGH>;
		$eleLine =~ s/^\s*//;
		$neighLine =~ s/^\s*//;
		my @verts = split /\s+/, $eleLine;
		my @neighs = split /\s+/, $neighLine;
		if ($verbose >= 2)
		{
			print "verts:\n";
			print Dumper(\@verts);
			print "neighs:\n";
			print Dumper(\@neighs);
		}
		push @potentialTriangles, {
			points => [ $points[$verts[1]], $points[$verts[2]], $points[$verts[3]] ],
			pointindices => [ $verts[1], $verts[2], $verts[3] ],
			neighbours => [ $neighs[1], $neighs[2], $neighs[3] ]
		};
	}
	close NODE;
	close ELE;
	close NEIGH;


	unlink($filetempname.".poly");
	unlink($filetempname.".1.node");
	unlink($filetempname.".1.ele");
	unlink($filetempname.".1.neigh");

	# the obvious O(n^2) searchy way is painfully slow on shapes with a few thousand tris/edges
	# prebuild a table here to our advantage. pointToEdge maps  ?
	my @pointToEdge;
	for (my $i = 0; $i <= $#$edges; $i++)
	{
		my $e = $edges->[$i];
		push @{$pointToEdge[$pointHash{$e->{from}}]}, $e;
		push @{$pointToEdge[$pointHash{$e->{to}}]}, $e;
	}

	print "pointToEdge: " . Dumper(\@pointToEdge) if $verbose;

	# go through all our edges and attach them to the triangles
	foreach my $tri (@potentialTriangles)
	{
		my $count = 0;

		for (my $i = 0; $i < 3; $i++)
		{
			my $p = $tri->{pointindices}[$i];
			my $e = $pointToEdge[$p];
			foreach my $edge (@$e)
			{
				my $count = 0;
				# determine how many times an point from our edge is in the triangle
				$count++ if ($tri->{pointindices}[0] == $pointHash{$edge->{from}});
				$count++ if ($tri->{pointindices}[0] == $pointHash{$edge->{to}});
				$count++ if ($tri->{pointindices}[1] == $pointHash{$edge->{from}});
				$count++ if ($tri->{pointindices}[1] == $pointHash{$edge->{to}});
				$count++ if ($tri->{pointindices}[2] == $pointHash{$edge->{from}});
				$count++ if ($tri->{pointindices}[2] == $pointHash{$edge->{to}});

				# if there's 2 point refs in this triangle, this edge is part of the triangle so we add a reference in this
				# triangle to the edge, as well as a reference to the point opposite the edge.
				if ($count >= 2)
				{
					my $opposite;
					$opposite = $tri->{points}[0] if ($tri->{pointindices}[0] ne $pointHash{$edge->{from}} && $tri->{pointindices}[0] ne $pointHash{$edge->{to}});
					$opposite = $tri->{points}[1] if ($tri->{pointindices}[1] ne $pointHash{$edge->{from}} && $tri->{pointindices}[1] ne $pointHash{$edge->{to}});
					$opposite = $tri->{points}[2] if ($tri->{pointindices}[2] ne $pointHash{$edge->{from}} && $tri->{pointindices}[2] ne $pointHash{$edge->{to}});

					die "couldn't find point opposite edge" unless defined($opposite);

					push @{$tri->{edgedata}}, {
						edge => $edge,
						oppositepoint => $opposite
					}
				}
			}
		}
	}
	undef @pointToEdge;

	print "points, potentialTriangles:\n" if $verbose;
	print Dumper(\@points, \@potentialTriangles) if $verbose;

	print "propagating fill styles...\n" if $verbose;

	# with all this stuff calculated, we can now determine the fill style for all of the triangles. we get triangle to generate a
	# "full" triangulation. this means that there's lots of triangles that are triangulating "empty space". here, we figure out
	# from the edges what fill (if any) a triangle has, and if it does have a fill, we add it to the list of actual triangles that
	# will appear in the output along with the actual fill that it will use.
	for (my $i = 0; $i <= $#potentialTriangles; $i++)
	{
		#print Dumper($potentialTriangles[$i]);
		my $edge = _findEdgeForTriangle($potentialTriangles[$i], \@potentialTriangles);
		print "edge:\n" if $verbose >= 2;
		print Dumper($edge) if $verbose >= 2;
		if (defined $edge)
		{
			my $fill;
			if (_isLeftFill($edge->{edge}{from}, $edge->{edge}{to}, $edge->{oppositepoint}))
			{
				$fill = $edge->{edge}{fill0};
			}
			else
			{
				$fill = $edge->{edge}{fill1};
			}
			if (defined $fill)
			{
				push @{$shape->{triangles}}, {
					a => $potentialTriangles[$i]->{points}[0],
					b => $potentialTriangles[$i]->{points}[1],
					c => $potentialTriangles[$i]->{points}[2],
					style => $fill
				};
			}
		}
	}

	print "num tris after all: " . ($#{$shape->{triangles}} + 1) . "\n" if $verbose;
}

sub _createLinearGradient($)
{
	my $Gradient = $_[0];
	my @imageData;
	my $curR = $Gradient->[0]->Color->Red;
	my $curG = $Gradient->[0]->Color->Green;
	my $curB = $Gradient->[0]->Color->Blue;
	my $curA = ref($Gradient->[0]->Color) =~ /RGBA/ ? $Gradient->[0]->Color->Alpha : 255;
	my $deltaR = 0;
	my $deltaG = 0;
	my $deltaB = 0;
	my $deltaA = 0;
	for (my $i = 0; $i < 256; $i++)
	{
		for (my $j = 0; $j <= $#{$Gradient}; $j++)
		{
			my $grad = $Gradient->[$j];
			if ($grad->Ratio == $i)
			{
				$curR = $grad->Color->Red;
				$curG = $grad->Color->Green;
				$curB = $grad->Color->Blue;
				$curA = ref($grad->Color) =~ /RGBA/ ? $grad->Color->Alpha : 255;

				if ($j < $#{$Gradient})
				{
					my $nextgrad = $Gradient->[$j+1];
					my $pixels = ($nextgrad->Ratio - $grad->Ratio);
					if ($pixels == 0)
					{
						$deltaR = $deltaG = $deltaB = $deltaA = 0;
					}
					else
					{
						$deltaR = ($nextgrad->Color->Red - $curR) / $pixels;
						$deltaG = ($nextgrad->Color->Green - $curG) / $pixels;
						$deltaB = ($nextgrad->Color->Blue - $curB) / $pixels;
						my $newA = ref($nextgrad->Color) =~ /RGBA/ ? $nextgrad->Color->Alpha : 255;
						$deltaA = ($newA - $curA) / $pixels;
					}
				}
				else
				{
					$deltaR = 0;
					$deltaG = 0;
					$deltaB = 0;
					$deltaA = 0;
				}
			}
		}

		$imageData[$i*4 + 0] = $curB;
		$imageData[$i*4 + 1] = $curG;
		$imageData[$i*4 + 2] = $curR;
		$imageData[$i*4 + 3] = $curA;
		$curR += $deltaR;
		$curG += $deltaG;
		$curB += $deltaB;
		$curA += $deltaA;
	}

	return @imageData;
}

sub buildShapes(%)
{
	my %params = @_;
	my %points;
	my $curLineStyleIndex = $params{curLineStyleIndex};
	my $curFillStyle0Index = $params{curFillStyle0Index};
	my $curFillStyle1Index = $params{curFillStyle1Index};
	my %shape;
	my @edges;
	my $last2Pos = undef;
	my $lastPos = _findEarliestPoint(0, 0, \%points);
	my @shapeList;

	my $shaperecs = $params{shaperecs};
	my $fills = $params{fills};
	my $lines = $params{lines};
	my $curLineStyle = defined($curLineStyleIndex) ? $lines->[$curLineStyleIndex] : undef;
	my $curFillStyle0 = defined($curFillStyle0Index) ? $fills->[$curFillStyle0Index] : undef;
	my $curFillStyle1 = defined($curFillStyle1Index) ? $fills->[$curFillStyle1Index] : undef;

	# moveto(0,0) is a special command that means that a new "layer" in the shape is starting. we run a triangulation pass in
	# this case right away.

	my $wantTriangulationNow = 0;

	for (my $shaperecIndex = 0; $shaperecIndex <= $#$shaperecs; $shaperecIndex++)
	{
		my $shaperec = $shaperecs->[$shaperecIndex];
		my $tagname = tagtype($shaperec);

		if ($tagname eq 'STYLECHANGERECORD')
		{
			if (!(ref($shaperec) =~ m/SHAPERECORD1/))
			{
				if ($#{$shaperec->FillStyles} >= 0)
				{
					$fills = $shaperec->FillStyles;
				}
				if ($#{$shaperec->LineStyles} >= 0)
				{
					$lines = $shaperec->LineStyles;
				}
			}

			if (defined $shaperec->LineStyle)
			{
				$curLineStyleIndex = $shaperec->LineStyle - 1;
				$curLineStyle = $shaperec->LineStyle > 0 ? $lines->[$curLineStyleIndex] : undef;
			}
			if (defined $shaperec->FillStyle0)
			{
				$curFillStyle0Index = $shaperec->FillStyle0 - 1;
				$curFillStyle0 = $shaperec->FillStyle0 > 0 ? $fills->[$curFillStyle0Index] : undef;
			}
			if (defined $shaperec->FillStyle1)
			{
				$curFillStyle1Index = $shaperec->FillStyle1 - 1;
				$curFillStyle1 = $shaperec->FillStyle1 > 0 ? $fills->[$curFillStyle1Index] : undef;
			}

			if (defined($shaperec->MoveDeltaX) || defined($shaperec->MoveDeltaY))
			{
				if ($shaperec->MoveDeltaX == 0 && $shaperec->MoveDeltaY == 0)
				{
					$wantTriangulationNow = 1;
					#Added following code for [ #618 ] APT exporter problem -> bitmap invisible in Apt-0.17.00
					#The problem was that the moveto( 0, 0) was being treated as a special case, but not handling an 
					#even more special case correctly.   Moveto(0,0) apparently represents a layer change.  
					#In this case the (0,0) coordinate was being ignored, which is correct most of the time, 
					#but sometimes this coordinate is actually used.  Usually there is a subsequent Moveto() with the starting 
					#coordinate for the next shape.  But if the starting coordinate for the next shape is (0,0) then a moveto 
					#is not used, because this has already been set.  So the solution is to use this 0,0 coordinate 
					#as the last coordinate entered, like in a regular moveto().
					$last2Pos = undef;
					$lastPos = _findEarliestPoint($shaperec->MoveDeltaX, $shaperec->MoveDeltaY, \%points);
				}
				else
				{
					$last2Pos = undef;
					$lastPos = _findEarliestPoint($shaperec->MoveDeltaX, $shaperec->MoveDeltaY, \%points);
				}
			}
		}
		elsif ($tagname eq 'STRAIGHTEDGERECORD')
		{
			my $newPos = _findEarliestPoint($lastPos->[0] + $shaperec->DeltaX, $lastPos->[1] + $shaperec->DeltaY, \%points);

			push @edges, { 
				from => $lastPos,
				to => $newPos,
				line => $curLineStyle,
				fill0 => $curFillStyle0,
				fill1 => $curFillStyle1
			};

			if (defined($last2Pos) && $newPos eq $last2Pos)
			{
				$edges[-1]->{fill0} = undef;
				$edges[-1]->{fill1} = undef;
				$edges[-2]->{fill0} = undef;
				$edges[-2]->{fill1} = undef;
			}

			$last2Pos = $lastPos;
			$lastPos = $newPos;
		}
		elsif ($tagname eq 'CURVEDEDGERECORD')
		{
			# tesselate the curve and add all points to the current curve
			my $cx = $lastPos->[0] + $shaperec->ControlDeltaX;
			my $cy = $lastPos->[1] + $shaperec->ControlDeltaY;
			my $ax = $cx + $shaperec->AnchorDeltaX;
			my $ay = $cy + $shaperec->AnchorDeltaY;
			my $controlPos = _findEarliestPoint($cx, $cy, \%points);
			my $anchorPos = _findEarliestPoint($ax, $ay, \%points);
			_buildBezierPoints(999, \@edges, \%points, $anchorPos, $controlPos, $lastPos, $curLineStyle, $curFillStyle0, $curFillStyle1);
#			push @edges, { 
#				from => $lastPos,
#				to => $anchorPos,
#				line => $curLineStyle,
#				fill0 => $curFillStyle0,
#				fill1 => $curFillStyle1
#			};
			$last2Pos = $lastPos;
			$lastPos = $anchorPos;
		}

		$wantTriangulationNow = 1 if ($shaperecIndex == $#$shaperecs);
		if ($wantTriangulationNow)
		{
			foreach my $edge (@edges)
			{
				if (defined $edge->{line})
				{
					push @{$shape{lines}}, {
						from => $edge->{from},
						to => $edge->{to},
						style => $edge->{line}
					};
				}
			}

#			print Dumper(\@edges);

			DefineShape_triangulate(\%shape, \@edges);
			$wantTriangulationNow = 0;
			push @shapeList, { %shape };
			undef %shape;
			undef @edges;
			undef %points;

		}
	}

	my %uniqueLinearStyle;
	my %uniqueRadialStyle;

	# convert gradients (linear ones right now) to bitmap fills
	foreach my $shape (@shapeList)
	{
		foreach my $triangle (@{$shape->{triangles}})
		{			
			if ($triangle->{style}->FillStyleType == 0x10) 	# linear gradient
			{
				my $frozenStyle = Storable::freeze(\$triangle->{style}->Gradient);
				
				# create a new gradient texture
				if (not $uniqueLinearStyle{$frozenStyle})
				{
					my $char = {
							type => 'texture',
					};
					$char->{id} = addAuxCharacter($char);
					$char->{tempfilename} = "$main::outdir{tex}\\" . $char->{id} . ".tga";

					my @imageData = _createLinearGradient($triangle->{style}->Gradient);

					my $data = pack('C*', @imageData);
					# sorry, ngc want the width/height of a texture at least be 4/4 if using 16/32 bit texture
					# for 8 bit texture width/height > 4/8, for 4 bit width/height > 8/8
					$data = $data x 4;
					#my $gradtex = Gimex::Create(256, 4, $data);
					#Gimex::Save($gradtex, $char->{tempfilename});
					AptImage::WriteTga32($char->{tempfilename}, 256, 4, $data);

					$uniqueLinearStyle{$frozenStyle} = $char->{id};
				}

				# now, overwrite the gradient with our fill
				$triangle->{style}->configure(BitmapID => $uniqueLinearStyle{$frozenStyle}, BitmapMatrix => $triangle->{style}->GradientMatrix);				
			}
			elsif ($triangle->{style}->FillStyleType == 0x12 or $triangle->{style}->FillStyleType == 0x13) 
			{
			    # 0x12 - radial gradient
			    # 0x13 - focal radial gradient (unsupported; treated same as radial gradient)
				my $frozenStyle = Storable::freeze(\$triangle->{style}->Gradient);
				
				# create a new gradient texture
				if (not $uniqueRadialStyle{$frozenStyle})
				{
					my $char = {
							type => 'texture',
					};
					$char->{id} = addAuxCharacter($char);
					$char->{tempfilename} = "$main::outdir{tex}\\" . $char->{id} . ".tga";

					my @imageData = _createLinearGradient($triangle->{style}->Gradient);

					$::GradientCircleSize = 32;		# make a nxn bitmap
					my @radialData;
					my $circleSize2 = ($::GradientCircleSize - 1) / 2;
					my $indexScale = 255 / $circleSize2;
					for (my $k=0; $k<$::GradientCircleSize; $k++)
					{
						my $y = $k - $circleSize2;
						for (my $j=0; $j<$::GradientCircleSize; $j++)
						{
							my $x = $j - $circleSize2;
							my $dist = sqrt($x*$x + $y*$y);
							my $index = $indexScale * $dist;

							$index = $index > 255 ? 255 : $index;

							$radialData[$k*$::GradientCircleSize*4 + $j*4 + 0] = $imageData[int($index)*4 + 0];
							$radialData[$k*$::GradientCircleSize*4 + $j*4 + 1] = $imageData[int($index)*4 + 1];
							$radialData[$k*$::GradientCircleSize*4 + $j*4 + 2] = $imageData[int($index)*4 + 2];
							$radialData[$k*$::GradientCircleSize*4 + $j*4 + 3] = $imageData[int($index)*4 + 3];
						}
					}

					my $data = pack('C*', @radialData);
					#my $gradtex = Gimex::Create($::GradientCircleSize, $::GradientCircleSize, $data);
					#Gimex::Save($gradtex, $char->{tempfilename});
					AptImage::WriteTga32($char->{tempfilename}, $::GradientCircleSize, $::GradientCircleSize, $data);

					$uniqueRadialStyle{$frozenStyle} = $char->{id};
				}

				# now, overwrite the gradient with our fill
				$triangle->{style}->configure(BitmapID => $uniqueRadialStyle{$frozenStyle}, BitmapMatrix => $triangle->{style}->GradientMatrix);	
			}
		}
	}

	return ( shapes => \@shapeList, curLineStyleIndex => $curLineStyleIndex, curFillStyle0Index => $curFillStyle0Index, curFillStyle1Index => $curFillStyle1Index, fills => $fills, lines => $lines );
}

sub getid_DefineShape4 { getid_DefineShape(@_) }
sub getid_DefineShape3 { getid_DefineShape(@_) }
sub getid_DefineShape2 { getid_DefineShape(@_) }
sub getid_DefineShape { return $_[0]->ShapeID }

sub build_DefineShape4 { build_DefineShape(@_) }
sub build_DefineShape3 { build_DefineShape(@_) }
sub build_DefineShape2 { build_DefineShape(@_) }
sub build_DefineShape
{
	my $element = shift;
	my @shapeList;

	my $shapes = $element->Shapes;

	my $fills = $shapes->FillStyles;
	my $lines = $shapes->LineStyles;
	my $shaperecs = $shapes->ShapeRecords();

	my %builtShapes = buildShapes(shaperecs => $shaperecs, fills => $fills, lines => $lines);

	return ({
		type => 'shape',
		boundingRect => [
			$element->ShapeBounds()->Xmin,
			$element->ShapeBounds()->Ymin,
			$element->ShapeBounds()->Xmax,
			$element->ShapeBounds()->Ymax],
		shapes => \@{$builtShapes{shapes}}
	}, $element->ShapeID);
}


