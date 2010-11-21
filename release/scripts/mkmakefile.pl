#!/usr/bin/env perl

# ************** <auto-copyright.pl BEGIN do not edit this line> **************
#
# VR Juggler is (C) Copyright 1998-2010 by Iowa State University
#
# Original Authors:
#   Allen Bierbaum, Christopher Just,
#   Patrick Hartling, Kevin Meinert,
#   Carolina Cruz-Neira, Albert Baker
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.
#
# *************** <auto-copyright.pl END do not edit this line> ***************

# This script can generate three types of template Makefile.in's from use with
# a configure script:
# 
#    1. A Doozer++ makefile that can do one of the following three build
#       processes:
#           A. Build object files from an auto-detected list of sources
#              coming from one or more user-specified directories
#           B. Recurse through a list of directories
#           C. A and B together
#    2. A makefile capable of compiling a single application from an
#       arbitrary auto-detected list of sources from an arbitrary list of
#       user-specified directories.
#    3. A makefile capable of compiling multiple applications from an
#       arbitrary list of user-specified sources (compared against an
#       auto-detected list for error checking) from an arbitrary list of
#       user-specified directories.
# 
# Given the right information, a "real" Makefile can be generated automatically
# for any of the above types as part of the creation process.
# 
# The makefiles generated can be used without modification, though in typical
# use, they will probably serve as more of a starting point requiring only
# minor modifications.

require 5.004;

use strict 'vars';
use vars qw($dir_prfx $exp_file $gmake $sub_objdir);
use vars qw(@basic_libs @extra_libs @includes);
use vars qw(%VARS);

use File::Basename;
use File::Copy;
use Getopt::Long;
use Text::Wrap qw(wrap $columns);

$columns = 76;

# Do this to include the path to the script in @INC.
my $path;

BEGIN {
    $path = (fileparse("$0"))[1];
}

use lib($path);
use InstallOps;
use SourceList;

# Subroutine prototypes.
sub printHelp();
sub findSources($$);
sub readSources($);
sub printObjMakefile($$@);
sub printAppMakefile($$$);
sub printMultiAppMakefile($$$);
sub printAppMakefileStart_in($$);
sub printAppObjs_in($$$);
sub printAppSuffixRules_in($$);
sub printAppMakefileEnd_in($;@);
sub expandAll($);

# Set default values for all variables used as storage by GetOptions().
my $app     = '';
my %apps    = ();
my $dotin   = 1;
my $gmake   = 0;
my $heading = '';
my $help    = 0;
my @srcdirs = ();
my @subdirs = ();
my @nosrcs  = ();

$dir_prfx   = '';
$sub_objdir = '';
@basic_libs = ();
@extra_libs = ();
@includes   = ();

my @saved_ARGV = @ARGV;

GetOptions('app=s' => \$app, 'apps=s' => \%apps, 'basiclibs=s' => \@basic_libs,
           'dirprefix=s' => \$dir_prfx, 'dotin!' => \$dotin,
           'expfile=s' => \$exp_file, 'extralibs=s' => \@extra_libs,
           'gmake' => \$gmake, 'heading=s' => \$heading, 'help' => \$help,
           'includes=s' => \@includes, 'nosrcs=s' => \@nosrcs,
           'srcdir=s' => \@srcdirs, 'subdirs=s' => \@subdirs,
           'subobjdir=s' => \$sub_objdir)
   or printHelp() && exit(1);

printHelp() && exit(0) if $help;

# This allows source directories to be specified using multiple options
# and/or using the form --<option>=opt1,opt2,...,optN.
@basic_libs = split(/,/, join(',', @basic_libs));
@extra_libs = split(/,/, join(',', @extra_libs));
@includes   = split(/,/, join(',', @includes));
@nosrcs     = split(/,/, join(',', @nosrcs));
@srcdirs    = split(/,/, join(',', @srcdirs));
@subdirs    = split(/,/, join(',', @subdirs));

my $all_src = findSources(\@srcdirs, \@nosrcs);

# Set the generated file name once since all the subroutines will create a
# file with the same name.
my $makefile_name  = 'Makefile';
$makefile_name    .= ".in" if $dotin;

# Create $makefile_name here.
open(MAKEFILE, "> $makefile_name")
   or die "ERROR: Could not create $makefile_name: $!\n";

if ( $heading )
{
   if ( open(HEADING, "$heading") )
   {
      print MAKEFILE while <HEADING>;
      close(HEADING);
   }
   else
   {
      warn "WARNING: Could not read from heading file $heading: $!\n";
   }
}

# Add a little commercial for this script.
print MAKEFILE "# Generated by mkmakefile.pl using the following options:\n";
print MAKEFILE wrap("#    ", "#    ", @saved_ARGV), "\n\n";

# Generate a single-application makefile.
if ( $app )
{
   printAppMakefile(MAKEFILE, $all_src, "$app");
}
# Generate a multi-application makefile.
elsif ( keys(%apps) )
{
   printMultiAppMakefile(MAKEFILE, $all_src, \%apps);
}
# Generate a Doozer++ object-building makefile.
else
{
   printObjMakefile(MAKEFILE, $all_src, @subdirs);
}

close(MAKEFILE) or warn "WARNING: Could not close $makefile_name: $!\n";

# Now replace all the @...@ strings if this is not a Makefile.in.
expandAll($makefile_name) unless $dotin;

exit(0);

# =============================================================================
# Subroutines follow.
# =============================================================================

# -----------------------------------------------------------------------------
# Print a usage message explaining how to use this script.
# -----------------------------------------------------------------------------
sub printHelp()
{
   print <<EOF;
Usage:
    $0
        [ --app=<name> | --apps <name1>=<srcs1> [ --apps <name2>=<srcs2> ... ]]
        [ --basiclibs=<lib1> [...] ] [ --dirprefix=<prefix> ]
        [ --dotin | --nodotin --expfile=<file> ] [ --extralibs=<lib1> [...] ]
        [ --gmake ] [ --heading=<file> ] [ --includes=<option> [...] ]
        [ --nosrcs=<file1> [...] ] [ --srcdir=<dirs> [...] ]
        [ --subdirs=<dirs> [...] ] [ --subobjdir=<dir> ]

For application makefiles:

    --app=<name>
        Specify the name of the application to be compiled.

  or

    --apps <name1>=<srcs1> --apps <name2>=<srcs2> ... --apps <nameN>=<srcsN>
        Specify the names of multiple applications and the lists of source
        files that will be compiled for each application.

    --basiclibs=<lib1> --basliclibs=<lib2> ... --basiclibs=<libN>
        Provide additional basic libraries that should be linked.  These
        are in addition to the default basic libraries linked.  Typically,
        these are optional libraries built as part of the system being
        compiled needed for a specific application.

    --extralibs=<lib1> --basliclibs=<lib2> ... --extralibs=<libN>
        Provide additional extra libraries that should be linked.  These
        are in addition to the default extra libraries linked.  Typically,
        these are optional system libraries needed for a specific application.

    --gmake
        Use GNU make features.

    --srcdir=<dirs1> --srcdir=<dirs2> ... --srcdir=<dirsN>
        Give a list of directories besides the current directory where
        source files will be found.  Each directory can be given as an
        argument to a separate --srcdir option, or they can all be passed
        to a single --srcdir option as a comma-separated list (no spaces).

For object makefiles:

    --dirprefix=<prefix>
        Name a prefix for the directory containing the source files.

    --subdirs=<dirs1> --subdirs=<dirs2> ... --subdirs=<dirsN>
        Give a list of subdirectories of the current directory where the
        build must recurse.  Each subdirectory can be given as an argument
        to a separate --subdirs option, or they can all be passed to a
        single --subdirs option as a comma-separated list (no spaces).

    --subobjdir=<dir>
        Name the subdirectory of \$(OBJDIR) where the object files will go.

For all types of makefiles:

    --dotin (default)
        Generate Makefile.in

  or

    --nodotin --expfile=<Perl file>
        Generate a fully expanded Makefile using the \%VARS hash in the
        file named as the argument to --expfile.

    --heading=<file>
        Print the contents of the given file at the top of the generated
        makefile before anything else is written.

    --includes=-I<path1> --includes=-I<path2> ... --includes=-I<pathN>
        Extend the include path to use the specified additional options.
        The options must be specified using the compiler option that
        extends the include path.  Each option can be given as an argument
        to a separate --includes option, or they can all be passed to a
        single --includes option as a comma-separated list (no spaces).

    --nosrcs=<file1> --nosrcs=<file2> ... --nosrcs=<fileN>
        List files that should be excluded when preparing the source list.
        Each file can be given as an argument to a separate --nosrcs option,
        or they can all be passed to a single --nosrcs option as a
        comma-separated list (no spaces).  Any paths used must be the same
        as used for --srcdir.
EOF
}

# -----------------------------------------------------------------------------
# Find all the source files in the given directories.
# -----------------------------------------------------------------------------
sub findSources($$)
{
   my @dirs   = @{$_[0]};
   my @nosrcs = @{$_[1]};

   # Make sure that the current directory is in the list of directories to
   # search.
   my $found_dot = 0;

   foreach ( @dirs )
   {
      if ( "$_" eq "." )
      {
         $found_dot = 1;
         last;
      }
   }

   push(@dirs, ".") unless $found_dot;

   # Create the primary SourceList object.  It will be populated with
   # information in the following loop.
   my $srcs = new SourceList();
   my $dir;

   # Loop over the given source directories.
   foreach $dir ( @dirs )
   {
      next if $srcs->hasDirectory("$dir");
      $srcs->readSources("$dir", @nosrcs)
         or warn "Failed to read sources in $dir\n";
   }

   return $srcs;
}

# -----------------------------------------------------------------------------
# Generate a makefile for use with Doozer++ that can compile object files and
# recurse into subdirectories if necessary.
# -----------------------------------------------------------------------------
sub printObjMakefile($$@)
{
   my $handle  = shift;
   my $srcs    = shift;
   my @subdirs = @_;

   my $include_dir = '@includedir@';
   $include_dir .= "/$dir_prfx" if $dir_prfx;

   # Print out the basic stuff that is common to all types of Doozer++
   # makefiles.
   print $handle <<MK_START;
default: all

# Include common definitions here.
#include ...

includedir	= $include_dir
srcdir		= \@srcdir\@
top_srcdir	= \@top_srcdir\@
INSTALL		= \@INSTALL\@
MK_START

   print $handle "EXTRA_INCLUDES\t+= @includes" if @includes;

   my($has_objs, $has_subdirs) = (0, 0);

   # Get a list of all the files in the directory.
   my @src_files = $srcs->getAllFiles();
   $has_objs = 1 if $#src_files > -1;

   print $handle "SUBOBJDIR\t= $sub_objdir\n" if $sub_objdir && $has_objs;

   print $handle "\n";

   # If there are subdirectories, list them.
   if ( $#subdirs > -1 )
   {
      $has_subdirs = 1;
      print $handle "DIRPRFX\t= $dir_prfx/\n\n" if $dir_prfx;

      print $handle "# Subdirectories to compile.\n";
      print $handle "SUBDIR\t=";

      foreach ( @subdirs )
      {
         print $handle " $_" unless "$_" eq "." ;
      }

      print $handle "\n\n";
   }

   # If there are object files to compile, list the source files.
   if ( $has_objs )
   {
      print $handle "SRCS\t= ", join(' ', sort(@src_files)), "\n";

      # If the directory list has more than one element (which would be
      # the current directory), add an assignment for $(EXTRA_SRCS_PATH) so
      # that make will know where to find all the source files.
      my @other_dirs = $srcs->getDirectories();
      if ( $#other_dirs > 0 )
      {
         print $handle "EXTRA_SRCS_PATH\t= ", join(' ', sort(@other_dirs)),
                       "\n";
      }

      print $handle "\n";
   }

   warn "No object files found, no subdirectories listed!\n"
      unless $has_objs || $has_subdirs;

   # Include the mk file that build objects and recurses if both object files
   # and subdirectories must be handled.
   if ( $has_objs && $has_subdirs )
   {
      print $handle "include \$(MKPATH)/dpp.obj-subdir.mk\n\n";
   }
   # If we have only objects to build, include that mk file.
   elsif ( $has_objs )
   {
      print $handle "include \$(MKPATH)/dpp.obj.mk\n\n";
   }
   # If we have only subdirectories, include the recursion file.
   elsif ( $has_subdirs )
   {
      print $handle "\$(RECTARGET): recursive\n\n";
      print $handle "include \$(MKPATH)/dpp.subdir.mk\n\n";
   }

   # If this directory has object files, generate the dependency including
   # code too.
   if ( $has_objs )
   {
      print $handle <<MK_END;
# -----------------------------------------------------------------------------
# Include dependencies generated automatically.
# -----------------------------------------------------------------------------
ifndef DO_CLEANDEPEND
-include \$(DEPEND_FILES)
endif
MK_END
   }
}

# -----------------------------------------------------------------------------
# Generate a makefile capable of compiling a single application.  This is
# relatively simple.
# -----------------------------------------------------------------------------
sub printAppMakefile($$$)
{
   my $handle = shift;
   my $srcs   = shift;
   my $app    = shift;

   print $handle "default: $app\@EXEEXT\@\n\n";

   printAppMakefileStart_in($handle, $srcs);
   printAppObjs_in($handle, 'OBJS', $srcs);

   # Print the target for the application.
   print $handle <<MK_END;
# -----------------------------------------------------------------------------
# Application build targets.
# -----------------------------------------------------------------------------
$app\@EXEEXT\@: \$(OBJS)
	\$(LINK) \@EXE_NAME_FLAG\@ \$(OBJS) \$(BASIC_LIBS) \$(EXTRA_LIBS)

MK_END

   printAppSuffixRules_in($handle, $srcs);
   printAppMakefileEnd_in($handle, "$app");
}

# -----------------------------------------------------------------------------
# Print a makefile that can compile multiple applications.  This would be
# simpler, but I put in some sanity checking that verifies that the source
# files listed on the command line actually exist.
# -----------------------------------------------------------------------------
sub printMultiAppMakefile($$$)
{
   my $handle = shift;
   my $srcs   = shift;
   my %apps   = %{$_[0]};

   print $handle "default: all\n\n";

   printAppMakefileStart_in($handle, $srcs);

   my @cur_srcs = ();
   my @all_apps = sort(keys(%apps));
   my $app;

   # Loop over all the applications and verify that their specific source
   # files exist.
   foreach $app ( @all_apps )
   {
      @cur_srcs = split(/,/, "$apps{$app}");

      # Create a new SourceList object for this application's source list.
      my $cur_app_src = new SourceList(@cur_srcs);

      # This the actual sanity check loop.
      my $file;
      foreach $file ( @cur_srcs )
      {
         # Find the current file in the known list.
         my $dir = $srcs->findFile("$file");

         # If it was found, insert it into the correct directory block for
         # the SourceList object.
         if ( $dir )
         {
            $cur_app_src->insertFile("$file", "$dir");
         }
         # If it was not found, complain.
         else
         {
            warn "WARING: Could not find file $file!\n";
         }
      }

      # Finally, generate the object list for this application.
      printAppObjs_in($handle, "${app}_OBJS", $cur_app_src);
   }

   print $handle <<EOF;
# -----------------------------------------------------------------------------
# Application build targets.
# -----------------------------------------------------------------------------
EOF

   print $handle "all:";

   foreach ( @all_apps )
   {
      print $handle " $_\@EXEEXT\@";
   }

   print $handle "\n\n";

   # Loop over the applications again and print the targets that build the
   # actual executables.
   foreach ( @all_apps )
   {
      print $handle <<MK_END;
$_\@EXEEXT\@: \$(${_}_OBJS)
	\$(LINK) \@EXE_NAME_FLAG\@ \$(${_}_OBJS) \$(BASIC_LIBS) \$(EXTRA_LIBS)

MK_END
   }

   printAppSuffixRules_in($handle, $srcs);
   printAppMakefileEnd_in($handle, @all_apps);
}

# -----------------------------------------------------------------------------
# Print the variable assignments and other "leading" information common to all
# applications to be compiled using this system.
# -----------------------------------------------------------------------------
sub printAppMakefileStart_in($$)
{
   my $handle = shift;
   my $srcs   = shift;

   my @dirs = sort($srcs->getDirectories());

   # Construct the complete list of include paths.
   my $inc_line = '@APP_INCLUDES@';
   $inc_line .= " @includes" if @includes;

   # Add each of the source directories to the include path.
   foreach ( @dirs )
   {
      if ( "$_" eq "." )
      {
         $inc_line .= ' -I$(srcdir)';
      }
      else
      {
         $inc_line .= " -I\$(srcdir)/$_";
      }
   }

   print $handle <<MK_START;
# Basic options.
srcdir		= \@srcdir\@
CFLAGS		= \@APP_CFLAGS\@ \$(EXTRA_CFLAGS) \$(INCLUDES) \$(DEFS)
CXXFLAGS	= \@APP_CXXFLAGS\@ \$(EXTRA_CFLAGS) \$(INCLUDES) \$(DEFS)
DEFS		= \@APP_DEFS\@
EXTRA_CFLAGS	= \@APP_EXTRA_CFLAGS\@ \$(DEBUG_CFLAGS)
DEBUG_CFLAGS	= \@APP_DEBUG_CFLAGS\@
OPTIM_CFLAGS	= \@APP_OPTIM_CFLAGS\@
INCLUDES	= $inc_line

EXTRA_LFLAGS	= \@APP_EXTRA_LFLAGS\@ \$(DEBUG_LFLAGS)
DEBUG_LFLAGS	= \@APP_DEBUG_LFLAGS\@
OPTIM_LFLAGS	= \@APP_OPTIM_LFLAGS\@
LINK_FLAGS	= \@APP_LINK_FLAGS\@ \$(EXTRA_LFLAGS)
LINKALL_ON	= \@APP_LINKALL_ON\@
LINKALL_OFF	= \@APP_LINKALL_OFF\@

# Libraries needed for linking.
BASIC_LIBS	= \@APP_BASIC_LIBS_BEGIN\@ \@APP_BASIC_LIBS\@ @basic_libs \@APP_BASIC_EXT_LIBS\@ \@APP_BASIC_LIBS_END\@
EXTRA_LIBS	= \@APP_EXTRA_LIBS_BEGIN\@ \@APP_EXTRA_LIBS\@ @extra_libs \@APP_EXTRA_LIBS_END\@ 

# Commands to execute.
C_COMPILE	= \@APP_CC\@ \$(CFLAGS)
CXX_COMPILE	= \@APP_CXX\@ \$(CXXFLAGS)
LINK		= \@APP_LINK\@ \$(LINK_FLAGS)

MK_START

   # Fill in the VPATH info.  We'll take advantage of GNU make syntax where
   # possible.
   if ( $gmake )
   {
      my($suffix, $dir);
      foreach $suffix ( $srcs->getSuffixes() )
      {
         print $handle 'vpath %', "$suffix ";

         foreach $dir ( @dirs )
         {
            if ( $srcs->hasSuffix("$dir", "$suffix") )
            {
               if ( "$dir" eq "." )
               {
                  print $handle '$(srcdir) ';
               }
               else
               {
                  print $handle "\$(srcdir)/$dir ";
               }
            }
         }

         print $handle "\n";
      }
   }
   else
   {
      print $handle "VPATH\t= \@srcdir\@";

      foreach ( @dirs )
      {
         print $handle ":\@srcdir\@/$_" if "$_" ne ".";
      }
   }

   # Finish the VPATH line.
   print $handle "\n\n";
}

# -----------------------------------------------------------------------------
# Print the assignment for the list of object files to be compiled.  This code
# looks pretty simple, but the ugliness is being hidden by the SourceList
# class.
# -----------------------------------------------------------------------------
sub printAppObjs_in($$$)
{
   my($handle, $obj_var, $srcs) = @_;

   # Get the list of files as a scalar and change the suffixes to @OBJEXT@.
   my $files = join(' ', sort($srcs->getAllFiles()));
   $files    =~ s/\.(cpp|cxx|c\+\+|cc|c)/.\@OBJEXT\@/gi;

   # XXX: Wrap me!
   print $handle "$obj_var\t= $files\n\n";
}

# -----------------------------------------------------------------------------
# Print the suffix rules needed for this appliation makefile.  Only the rules
# needed for the known sources are generated to keep the file simple.
# -----------------------------------------------------------------------------
sub printAppSuffixRules_in($$)
{
   my($handle, $srcs) = @_;

   my @suffixes = sort($srcs->getSuffixes());

   print $handle "# Suffix rules for building object files.\n";
   print $handle ".SUFFIXES: ", join(' ', @suffixes), " .\@OBJEXT\@\n\n";

   # XXX: Could use GNU stuff too.
   foreach ( @suffixes )
   {
      print $handle "$_.\@OBJEXT\@:\n";

      # C source file.
      if ( "$_" eq ".c" )
      {
         print $handle "\t\$(C_COMPILE) \@OBJ_NAME_FLAG\@ \@OBJ_BUILD_FLAG\@ \$<\n\n";
      }
      # C++ source file.
      else
      {
         print $handle "\t\$(CXX_COMPILE) \@OBJ_NAME_FLAG\@ \@OBJ_BUILD_FLAG\@ \$<\n\n";
      }
   }
}

# -----------------------------------------------------------------------------
# Finish off the application makefile by adding 'clean' and 'clobber' targets.
# -----------------------------------------------------------------------------
sub printAppMakefileEnd_in($;@)
{
   my $handle = shift;
   my @apps   = @_;

   my($dirt, $cruft) = ('', '');
   foreach ( @_ )
   {
      $dirt  .= "$_\@EXEEXT\@ ";
      $cruft .= "$_.ilk ";
   }

   print $handle <<MK_END;
# -----------------------------------------------------------------------------
# Clean-up targets.
# -----------------------------------------------------------------------------
clean:
	rm -f Makedepend *.\@OBJEXT\@ $cruft so_locations *.?db core*
	rm -rf ii_files

clobber:
	\@\$(MAKE) clean
	rm -f $dirt
MK_END
}

# -----------------------------------------------------------------------------
# Expand all the @...@ strings using the %VARS hash.
# -----------------------------------------------------------------------------
sub expandAll($)
{
   my $filename = shift;

   # Suck in the full %VARS hash.
   require "$exp_file" if $exp_file;

   warn "WARNING: No expansion values!\n" unless keys(%VARS) && $exp_file;

   replaceTags("$filename", %VARS);
}
