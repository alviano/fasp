Preliminary steps
====================

Patches for parsing numeric constants in gringo are in the gringo directory.
Apply all of them to the respective files. You must rebuild parser files, which 
may be tricky:

- Compile lemon/lemon.c with gcc (to obtain a binary lemon).

- Enter directory libgringo/src.

- Run re2c on parser.r2c and save the output on parser.cpp.

- Run lemon on parser_impl.y; then move parser_impl.c into parser_impl.cpp.

Now compile gringo as usual.


To compile fasp just run make in the src directory. This will allow to compute
approximations of fuzzy answer sets or the well-founded semantics. If you are
interested in the computation of fuzzy answer sets (under the Lukasiewicz
t-norm), you need the YALMIP library:

http://users.isy.liu.se/johanl/yalmip/pmwiki.php?n=Main.Download

and to install Octave:

http://www.gnu.org/software/octave/

Moreover, YALMIP has to be patched (the patch is in the yalmip directory) and
you have to specify the location of the patched code into .octaverc (to be
placed in your home directory). An example of this file is available in the
yalmip directory and reported below:

<.octaverc>
do_braindead_shortcircuit_evaluation (1);

warning("off", "Octave:possible-matlab-short-circuit-operator");
warning ("off", "Octave:shadowed-function");

addpath(genpath('PATH TO YALMIP'));

function varargout = ismembc (varargin)
    varargout = cell (nargout, 1);
    [varargout{:}] = ismember (varargin{:});
endfunction
</.octaverc>

Replace PATH TO YALMIP with the path to the library in your machine.


Usage
======

Provide an encoding to gringo and pipe the output of gringo (numeric format) to
fasp, e.g.

gringo example.fasp | fasp

Invoke fasp with command-line option --help for details.

