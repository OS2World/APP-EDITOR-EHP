.* maximum res id: 31
:userdoc.
:title.EHP online manual

.* ************************************************************************
:h1.Introduction
.* ************************************************************************

:p. First of all note, that since release 1.4 :hp4.EHP:ehp4. is shareware. If
you are using it for longer than four weeks you will have to 
:link reftype=hd res=31. register:elink.. Please DO register. :hp4.EHP:ehp4. 
is not crippled in anyway and I expect the appropriate fairness from you. The
registration fee is a joke compared against the efforts needed to port the 
editor to OS/2 and maintaining it.

:p.The shortcut :hp4.EHP:ehp4. comes from "Editor for HP workstation" and was
invented during the first implementation on a HP 350. This first 
implementation was done by Maximilian Riesenhuber and me during 1990.
Since then :hp4.EHP:ehp4. has gone its way via DOS to OS/2. Of course the
name remains only for historical reasons, since it has no real meaning
for the editor now running with OS/2.

:p.:hp4.EHP:ehp4. is an
interactive fullscreen text editor, running also in a PM text window and
allowing quasi-parallel editing of multiple texts using window technique. 
3-button mouse support is provided (of course only, if a 3-button mouse 
driver is installed).

:p.Compared with the UNIX vi, :hp4.EHP:ehp4. does not distinguish an edit- and a
command mode. Instead, the control- and the alt-keys are used for invoking
commands, as most today-editors do.

Why use :hp4.EHP:ehp4. instead of e.g. EPM or other PM editors? Of course, 
discussing this topic is a very subjective issue, and we're aware, that 
people don't change their favourite editor like others change their shirt. 
The first reason is, that :hp4.EHP:ehp4. starts up more quickly than EPM. 
:hp4.EHP:ehp4., as mentioned and runs also full-screen, which makes it a
possible choice for system recovery, if PM doesn't come up. Furthermore,
:hp4.EHP:ehp4. can be - and has been - easily ported to other platforms like 
UNIX, since it was implemented upon the curses library initiallly. In some 
way it has more powerful features than, say, EPM. E.g. the possibilty of 
intermixing macros and repeat commands and nesting them into each other 
might be possible only with EMACS. By the way one more word on EMACS: Though 
I'm not using it, I'm sure it's a very powerful editor. But the torture of 
installing and learning it, the gigantic ressource comsumption, the overhad
of having a complete LISP system while not really using it and so on make 
EMACS not the first choice for everybody.

As a quick overview, we might give a list of the distinguishing features
of :hp4.EHP:ehp4.&colon.
:ul.
:li.Interactive 32-bit text editor, well-suited for programming or writing
TeX
:li.Runs OS/2 fullscreen or in PM text window in different text resolutions
(80x25, 80x50, etc.)
:li.Windowed user interface with mouse support
:li.Keybindings redefinable
:li.Full-featured regular expressions for search
:li.Powerful macro and repeat commands
:li.Powerful block commands
:li.Support for rectangular blocks (wonderful for maintaining comments)
:li.Mode with automatic linebreaking and paragraph reflowing
:li.Remembers its configuration exactly when exiting
:li.Copes with underlined characters (Unix-style: _^H) and can easily
manage them
:li.Comes with detailed online documentation in .INF format
:eul.

:p.In the sequel all available commands will be listed in a semantic
grouping. For a command reference see 
:link reftype=hd res=1.Command reference:elink. in the apppendix.

:p.Note, that when this document speaks about a 'screen', and you are
running :hp4.EHP:ehp4. in an OS/2 PM text window, then screen means that
window. Don't intermix PM windows with :hp4.EHP:ehp4. windows!

:p.Note, that the following words are sometimes used as
synonyms:
:ul.
:li.:hp2.RETURN:ehp2. and :hp2.ENTER:ehp2.
:li.:hp2.Space:ehp2. and :hp2.blank:ehp2.
:li.:hp2.Tab:ehp2. and :hp2.tabstop:ehp2.
:eul.

.* ************************************************************************
:h1 res=31.Registration
.* ************************************************************************

:p.If you are using :hp4.EHP:ehp4. for more than four weeks you will have to 
register. Please register by sending $10 or DM 10,- and your EMail address to

:lines align=left.
       Axel Uhl
       Weinbrennerstraáe 108
       76185 Karlsruhe
       Germany
:elines.

:p.This will yield you an unlimited license of using all further releases of
:hp4.EHP:ehp4.. Additionally, it will provide you the advantage of receiving 
the latest updates via UUENCODED EMail (please write if you don't need this, 
e.g. due to EMail cost).

:p.To state it once again: :hp4.EHP:ehp4. is completely uncrippled. Please 
note this, be equally fair and register if you are really using this editor. 
10$ is nothing for an editor with :hp4.EHP:ehp4.'s features.

.* ************************************************************************
:h1 res=3.Installing EHP
.* ************************************************************************

:p.The installation of :hp4.EHP:ehp4. is quite simple. Place the file
EHP.EXE in a directory contained in your :hp2.PATH:ehp2., e.g. 
c&colon.\os2\bin.
Place the keyboard definition file tasten.inf in a directory of your
choice, say also c&colon.\os2\bin. Edit your :hp2.CONFIG.SYS:ehp2. file and
insert the line :hp2.SET EHPINIT=keys c&colon.\os2\bin\tasten.inf:ehp2.. This
forwards the path of the keyboard file to :hp4.EHP:ehp4.. For further
settings via :hp2.EHPINIT:ehp2. see :link reftype=hd res=2.EHPINIT:elink..
Further, the dynamic link library EMX.DLL must be in a directory contained
in your :hp2.LIBPATH:ehp2..

.* ************************************************************************
:h1.General information
.* ************************************************************************

:p.This section contains information about invoking :hp4.EHP:ehp4., the
contents of the screen, user interaction and accepted fileformats.

.* ************************************
:h2.Invoking EHP
.* ************************************

:p.Precondition for invoking :hp4.EHP:ehp4. is the correct 
:link reftype=hd res=3.installation:elink. of :hp4.EHP:ehp4..
If this is done, you can start the editor by typing :hp2.ehp:ehp2. at
the command prompt. You can provide the names of files to load as
parameters. E.g. the command 

:p.:hp2.ehp x y z:ehp2. 

:p.loads the files
x, y and z each into a seperate window. The window displayed on top of
the others is the one named first, in this case x, since files are loaded
in reverse order, and each newly opened window becomes the top window.
Note, that the numbering of the windows is also in reverse order.
The numbering can be used to 
:link reftype=hd res=5.switch among the windows:elink..

:p.If no parameters were provided, :hp4.EHP:ehp4. looks for a
:link reftype=hd res=4.configuration file:elink.. If such a file exists,
the editor is restored in the state it was in when quitting it before.
Otherwise you are asked for the name of a file to load. If a file with
that name does not exist, you are asked whether you want to create it.
If so, a new empty file is presented. Otherwise the editor terminates
with a corresponding message.

.* ************************************
:h2.Screen contents
.* ************************************

:p.This section describes the outfit of windows, the position and the
contents of the status line and the colors.

.* ************************************
:h3.Windows
.* ************************************

:p.A window is a rectangular part of the screen, on which text is
presented in a frame. Since it is possible to look at multiple windows
at the same time, the current window's frame is highlighted.
The frame contains helpful information and
:link reftype=hd res=11.control areas for the mouse:elink..
:p.

.* ************************************
:h4.Headline
.* ************************************

:p.In the headline you find the current line and column number,
information about :link reftype=hd res=6.insert/overwrite:elink. 
(:hp2.Insert:ehp2. or :hp2.Overwrite:ehp2.),
:link reftype=hd res=7.underline:elink. (:hp2.Under:ehp2.), 
:link reftype=hd res=9.autoindent:elink. (:hp2.Indent:ehp2.),
:link reftype=hd res=8.shell:elink. (:hp2.SHELL:ehp2.)
and :link reftype=hd res=9.tab:elink. (:hp2.Tabs:ehp2. or :hp2.Spcs:ehp2.)
mode.

:p.

.* ************************************
:h4.Footline
.* ************************************

:p.In the footline the window number and the filename are displayed.
If the file is read only, then :hp2.WRITEPROTECTED:ehp2. is displayed.
If the file was modified after loading or after the last saving, then
:hp2.MODIFIED:ehp2. is displayed in the footline. Such modified files are
saved when :link reftype=hd res=10.quitting the editor:elink. with the save
option (of course only, if the file was not write protected).

.* ************************************
:h3 res=12.Status line
.* ************************************

:p.The status line is the bottom line of the screen. Here the helptexts
for the keycommands and error messages are displayed.

.* ************************************
:h3 res=24.Colors
.* ************************************

:p.The default color setting is the following:
:ul.
:li.Grey
:lp.This is the color normal text and frames of inactive windows appear in.
:li.Green
:lp.Highlighted blocks and the frame of the current window appear green.
:li.Red
:lp.With red underlined text passages are displayed.
:li.Brown
:lp.If underlined text appears in a highlighted block, it is colored brown.
:eul.

.* ************************************
:h2.Interaction
.* ************************************

:p.An input consisting of more than one letter (except the key commands)
has to be terminated with the :hp2.ENTER:ehp2.-key. This are for example
filenames or line numbers. If :hp4.EHP:ehp4. expects exactly one letter
(for example in a yes/no query), pressing the letter suffices and
pressing :hp2.ENTER:ehp2. is not needed.

:p.Error messages, which are displayed in the
:link reftype=hd res=12.status line:elink. must be confirmed with 
:hp2.ENTER:ehp2..
If a hint is displayed in the status line, e.g. the new status of the
the status of a binary variable, like the blockhighlighting status,
this hint will disappear automatically after a couple of seconds.

:p.The keyboard commands are case sensitive. That means, if e.g. in the
:link reftype=hd res=1.command reference:elink. there is a command
:hp2.CTRL-x n:ehp2. then you have to use a lower-case n. Note, that hence
having Caps Lock activated accidently will cause troubles in executing
commands.

.* ************************************
:h2 res=23.Fileformat
.* ************************************

:p.The files to edit must be in ASCII format. With OS/2 the size of texts
is limited to some GB, which will satisfy most needs and will exceed
swap space in most cases. However, if a file should still be too large to
load, an error message will be displayed.

:p.If lines longer than 500 characters appear, they will be wrapped (see
also :link reftype=hd res=13.Limits and bugs:elink.). The limit on the
number of lines is 4G due to the limit of integers, but this has no effect,
since memory limits are more strict.

:p.Note, that binary files should not be edited with :hp4.EHP:ehp4., since
e.g. :link reftype=hd res=9.tabs are expanded:elink. during load, and are 
only recompressed at the
beginning of a line (important for tools like :hp2.MAKE:ehp2.). This could
lead to an unexpected and unpleasant change in representation.

:p.A combination of an underscore followed by a backspace and a character is
interpreted as if that character is underlined. Characters entered in
:hp4.EHP:ehp4.'s :link reftype=hd res=7.underline mode:elink. are
written to file in the same representation. Due to this representation,
if manually an underscore followed by a backspace (entered via
:hp2.CTRL-c Backspace:ehp2.) is inserted before a character, this
character will appear underlined in the sequel. This is also true for
an underscore-backspace combination entered at the end of a line, which
will be converted into an underlined space.

.* ************************************************************************
:h1.Commands
.* ************************************************************************

:p.The keyboard mapping for commands was chosen in a way that key
combinations can be remembered as easily as possible. However, a compromise
had to be found between shortness of commands and the mnemonic property.
So for heavily used commands sometimes in the predefined keyboard mapping
a somewhat cryptical combination has been chosen to speed up everyday's
work (e.g. :hp2.CTRL-t:ehp2. for 'delete word').

:p.The key combinations for invoking the commands may be changed by using
the withcoming tool :hp4.KM:ehp4., explained below (see 
:link reftype=hd res=30.Keyboard remapping:elink.).

:p.Note, that if typing an illegal combination, all typed keys belonging
to the current command are discarded up to and including the one causing
the sequence to be illegal. Say for example you wanted to type the command
for deleting a block (:hp2.CTRL-b d:ehp2.) but you hit s instead of d, then 
the :hp2.CTRL-b:ehp2. and the s are discarded. This can of course be used
as a kind of escape facility. E.g. the spacebar is currently not assigned
any command. So pressing :hp2.SPACE:ehp2. escapes from a multi-key command.

.* ************************************
:h2.Moving text and cursor
.* ************************************
:p.

:h3 res=19.Change cursorposition

:p.In order to position the cursor there are many commands. All have in 
common that they can't move the cursor beyond the last line of the text
and beyond column 501 (see :link reftype=hd res=13.Limits and Bugs:elink.).
If the cursor would move out of the window, the text is scrolled accordingly.
:p.The following sections will describe the single commands in detail.

:h4.Character oriented
:ul.
:li.:hp2.Arrow-left:ehp2.
:lp.moves the cursor one column left, if not already in column 1. If in
column 1, then the cursor is moved to the end of the previous line, if
the current line number was greater than 1.
:li.:hp2.Arrow-right:ehp2.
:lp.moves the cursor one column right, if not already in column 501.
:eul.

.* ************************************
:h4.Word oriented
.* ************************************

:p.Word separator are the following characters:
:hp2..,#&colon.;]@$&amp.()[]{}!?'`/\<>-+*=":ehp2. in addition to 
:hp2.TAB:ehp2., :hp2.SPACE:ehp2. and a line break.
:ul.
:li.:hp2.CTRL-Arrow left:ehp2.
:lp.moves the cursor one word left, if not already at the beginning of
the line. Consecutive word separators (except if separated by
blanks or tabs) are treated as one word, which makes word skipping much
more convenient. By the way: this is the way, VI does word skipping, too.
:li.:hp2.CTRL-Arrow right:ehp2.
:lp.moves the cursor one word right, if not already at the last word of
the line. As stated above, consecutive word separators not separated by
blanks or tabs are treated as one word. If the cursor is placed on the
last word in a line, this function places the cursor immediately right to
the end of the line (as if the :hp2.End:ehp2.-key had been pressed).
:eul.

.* ************************************
:h4.Line oriented
.* ************************************

:ul.
:li.:hp2.Home:ehp2.
:lp.moves cursor to the beginning of the current line.
:li.:hp2.End:ehp2.
:lp.moves cursor to the end of the current line.
:li.:hp2.Arrow up:ehp2.
:lp.moves cursor up one line, if not already in first line of current file.
:li.:hp2.Arrow down:ehp2.
:lp.moves cursor down one line, if not already in last line of current file.
:li.:hp2.ENTER:ehp2. (in overwrite mode, see 
:link reftype=hd res=6.Insert and delete:elink.)
:lp.moves cursor to the beginning of the following line, if the current
line is not the last line of the current file.
:eul.

.* ************************************
:h4.Window oriented
.* ************************************

:ul.
:li.:hp2.CTRL-Home:ehp2.
:lp.moves the cursor to the top of the current window. Note that this need
not be the first line of the file.
:li.:hp2.ALT-Arrow right:ehp2.
:lp.move the cursor to the middle line of the current window. If the last
line of the file is displayed above the middle of the window or the window
is empty, then the cursor is placed to the last line of the file.
:li.:hp2.CTRL-End:ehp2.
:lp.moves the cursor to the bottom of the current window. Note that this
need not be the last line of the current file. If the last line of the
file is displayed above the bottom of the window, then the cursor is
places in the last line of the file.
:eul.

.* ************************************
:h3.Change window contents
.* ************************************
:p.

:h4.Line oriented
:ul.
:li.:hp2.ALT-Arrow up:ehp2.
:lp.makes current line the top line of the current window.
:li.:hp2.ALT-Arrow left:ehp2.
:lp.makes current line the middle line of the current window.
:li.:hp2.ALT-Arrow down:ehp2.
:lp.makes current line the bottom line of the current window.
:li.:hp2.CTRL-Arrow up:ehp2.
:lp.moves window content up one line, if the current line is not the last
line of the current file and is located at the top of the window.
The cursor stays in the same line number, if it was not in the top line
of the window. If the latter was the case, then it is moved down to the
next line. That means it stays on the same screen position in that case.
:li.:hp2.CTRL-Arrow down:ehp2.
:lp.moves window content down one line, if the first line on the current
file is not already visible in the window.
The cursor stays in the same line number, if it was not in the bottom line
of the window. If the latter was the case, then it is moved up to the
next line. That means it stays on the same screen position in that case.
:eul.

.* ************************************
:h4.Page oriented
.* ************************************

:ul.
:li.:hp2.PgUp:ehp2.
:lp.scrolls up one page, if not already at the beginning of the file.
The direction is hence towards smaller line numbers.
:li.:hp2.PgDn:ehp2.
:lp.scrolls down one page, if not already at the end of the file. 
The direction is hence towards greater line numbers.
:li.:hp2.ALT-PgUp:ehp2.
:lp.sctolls up half a page, if not already at the beginning of the file.
The direction is hence towards smaller line numbers.
:li.:hp2.ALT-PgDn:ehp2.
:lp.scrolls down half a page, if not already at the end of the file.
The direction is hence towards greater line numbers.
:eul.

.* ************************************
:h3.Absolute jumps
.* ************************************

:ul.
:li.:hp2.CTRL-PgUp:ehp2.
:lp.moves the cursor to the first line of the current file.
:li.:hp2.CTRL-PgDn:ehp2.
:lp.moves the cursor to the last line of the current file.
:li.:hp2.CTRL-g n:ehp2.
:lp.go to a specific line number. The query for the line number is
displayed in the :link reftype=hd res=12.status line:elink..
:li.:hp2.CTRL-g b:ehp2.
:lp.moves cursor to the beginning of the block marked in the current window.
If no block is marked in the current window, then the corsorposition remains
unchanged.
:li.:hp2.CTRL-g e:ehp2.
:lp.moves cursor to the end of the block marked in the current window.
If no block is marked in the current window, then the corsorposition remains
unchanged.
:li.:hp2.CTRL-g l:ehp2.
:lp.move the cursor to the so-called 'last position'. This position is
saved, when a jump to the beginning or the end of a file, a search, a replace
or any of the :hp2.CTRL-g:ehp2. commands is performed. By the latter it is
possible to jump back and forth between two positions by repeatedly doing
:hp2.CTRL-g l:ehp2..
:li.:hp2.Left mouse button:ehp2.
:lp.By moving the mouse pointer to a suitable position and pressing the
left mouse button once, the cursor is moved to the mouse position.
Note, that this also works, if the mouse pointer is located outsinde the 
current window (see :link reftype=hd res=5.window switching:elink.).
:li.:hp2.CTRL-p:ehp2.
:lp.move the cursor to the bracket or parenthesis matching the one
being nearest to the cursor. This command is extremely useful for testing
the matching of nested parenthesis / curly braces as they appear in C or 
in LISP. If no matching parenthesis / bracket is found, :hp4.EHP:ehp4.
issues a beep.
:eul.

.* ************************************
:h3.Marker

:p.There is a total of ten markers which may be set anywhere in open 
windows. Markers are global, which means, that a marker not only contains 
the cursor line and column, but also the window index. This may be helpful 
for performing fast window switching, since of course the window of the 
marker jumped to is made to be the current window.

:ul.
:li.:hp2.CTRL-g s <n>:ehp2.
:lp.sets marker number <n> at the current position.
:li.:hp2.CTRL-g g <n>:ehp2.
:lp.goes to marker number <n> and activates the corresponding window.
:eul.

.* ************************************
:h2.File operations
.* ************************************

:p.The following operations always refer to the current window and to
the whole file in the window.

:ul.
:li.:hp2.CTRL-e l:ehp2.
:lp.loads a file into the current window. The file in the current window,
and all unsaved changes applied to it, are discarded after a security query.
The name of the file to load is asked for in the 
:link reftype=hd res=12.status line:elink.. If a file with that name is
already contained in another open window, then this is announced and
one can choose to edit that window or nevertheless load the file in the
current window. Note, that the latter causes two possibly different copies
of the same file to be held in :hp4.EHP:ehp4., which could lead to loss
of changes.
:li.:hp2.CTRL-e s:ehp2.
:lp.saves the file contained in the current window. If the 
:link reftype=hd res=14.backupoption:elink. is activated, the file, which
would be going to be overwritten, gets the extension '.bak'. File attributes
(system, hidden etc.) remain as they were when loading the file. New files
are created as non-system, non-hidden and writeable.
:li.:hp2.CTRL-e r:ehp2.
:lp.renames the file and saves it under the new name. The new file name is
asked for in the status line. Note, that the file with the old name is
not deleted. Subsequent saving causes the file to be saved with the new
name.                            
:li.:hp2.CTRL-e m:ehp2.
:lp.saves all modified files. Files not modified are not written to disk
on this command.
:eul.

.* ************************************
:h2 res=15.Block operations
.* ************************************

:p.:hp4.EHP:ehp4. offers a great variety of block commands. It is important
to know, that :hp4.EHP:ehp4. supports two kinds of blocks. There are
the so-called :hp1.rectangular blocks:ehp1.. They are defined by marking
the upper left corner and the character right to the lower right corner.
The rectangular field between is the block. These blocks are nice to move
parts of tables or comments in a program.

:p.The second block kind is the so-called :hp1.normal block:ehp1.. One
marks the first character and the character right to the last character.
All the text between (with line wrapping etc.) belongs to the block.
These blocks are good for moving whole paragraphs or parts of paragraphs.

.* ************************************
:h3.Updating of block boundaries
.* ************************************

:p.When inserting lines before a block, then the beginning and the end
of the block are moved correspondingly. So the text does not move through
the block, but the marked block moves and the block contents stay the same.
When a line is inserted within the block, then the block end moves one line
down. When characters are inserted or deleted within a block, then the 
resulting block changes depend on the block kind.
:p.For normal blocks if the character was inserted/deleted in the last block 
line (i.e. the block end is on the current line), then the block end moves 
right/left one position.
:p.For rectangular blocks the block remains rectangular with the same width
all the time. That means, that characters may be moved outside/into the
block.

.* ************************************
:h3.Blockhighlighting
.* ************************************

:p.If a block was correctly marked, i.e. the block end comes after the
block beginning (though you can mark them in any order) and the
:link reftype=hd res=14.blockhighlighting is activated:elink., then
the block is displayed in a color different from the standard text color.
The blockhighlighting can also be turned off via the environment variable
:link reftype=hd res=2.EHPINIT:elink..

.* ************************************
:h3.Block oriented commands
.* ************************************

:p.Note, that the following commands perform actions on blocks local to
the current window except the commands 'Cut' and 'Paste'. Only with these
two commands it is possible to move blocks between windows.

:ul.
:li.:hp2.CTRL-b b:ehp2.
:lp.marks the beginning of a block. The character the cursor is on is the
first one belonging to the block.
:li.:hp2.CTRL-b n:ehp2.
:lp.marks the end of a :link reftype=hd res=15.normal block:elink.. The 
current character is the first one not belonging to the block.
:li.:hp2.CTRL-b r:ehp2.
:lp.marks the end of a :link reftype=hd res=15.rectangular block:elink.. The
current character is the first one not belonging to the block.
:li.:hp2.CTRL-b u:ehp2.
:lp.unmarks the block. That means, that block beginning and block end are
unmarked and that, if it was activated, the blockhighlighting disappears.
:li.:hp2.CTRL-b c:ehp2.
:lp.copies the marked block to the current cursor position. The block is
inserted, so no characters are overwritten. In case of a rectangular block
a check is performed, if insertion of the block does not collide with 
the maximum line length. If it does, then the block is not inserted.
If no block was marked, nothing happens.
:li.:hp2.CTRL-b m:ehp2.
:lp.moves the marked block to the cursor position. As with copying, a
check is done to see, if a rectangular block does not collide with the
maximum line length, if inserted at the cursor position. It is not moved,
if it does collide. If no block was marked, nothing happens.
:li.:hp2.CTRL-b d:ehp2.
:lp.deletes the marked block. You can undo this command by using the
:hp3.block copy:ehp3. command :hp2.CTRL-b c:ehp2.. But note, that this
undoing only works as long as no new block boundary has been set.
Furthermore, the buffered deleted block is discarded when exiting
:hp4.EHP:ehp4. and is not stored in the :link reftype=hd res=4.configuration
file:elink..
:li.:hp2.CTRL-b i:ehp2.
:lp.indents the block by a certain amount of columns to the right or left.
The number of columns is queried in the 
:link reftype=hd res=12.status line:elink.. Providing a positive number
shifts the block to the right, a negative number to the left. Note, that
a negative number with an absolute value greater than the maximum number
of leading spaces in the block leads to loss of characters in such lines,
since they are shifted 'outside the window'. All block lines including block
starting line and block ending line are shifted, except when the block end
is on the first column of a line. Then this line is not shifted left, since
no character of that line belongs to the block. Block start and end column
are shifted also, except the block end column was in the first column. In
this case indenting the block will leave the block end column unchanged.
This will cause the line, where the block end is set, not to fall into
the block, which would be surprising, when indenting the same block again.
:li.:hp2.CTRL-b r:ehp2.
:lp.reads a file, of which the name is asked for in the 
:link reftype=hd res=12.status line:elink. and inserts it at the cursor 
position. The file contents are marked as a 
:link reftype=hd res=15.normal block:elink..
:li.:hp2.CTRL-b w:ehp2.
:lp.writes the marked block to a file, of which the name is asked for
in the status line. This works for 
:link reftype=hd res=15.normal and rectangular blocks:elink..
:li.:hp2.CTRL-b f:ehp2.
:lp.passes the block contents to an external filter (e.g. :hp2.SORT:ehp2.),
deletes the block and inserts the filter outputs as a new block at the
position of the old one. If the filter returns with a non-zero value,
one can decide, whether the results should nevertheless be inserted as block
or if they should be discarded. The former can be useful for programs,
which are known to return non-zero values even if no error occurred.
:lp.Note, that the name of the used filter serves as default for the next
call. Thus the previously used filter name is displayed in the prompt for
the new filter name. If one wants to use the default, one just has to
press :hp2.ENTER:ehp2. without providing the filter name.
:li.:hp2.CTRL-b t:ehp2.
:lp.cuts the block. This is not a cut in the common sense, since it has
become a standard for cut operations to delete the cutted part. This cut
only copies the marked block into the so-called 'paste buffer'.
The contents of the paste buffer can be inserted in any
window, as described in the sequel.
:li.:hp2.CTRL-b p:ehp2.
:lp.pastes the contents of the paste buffer into the current window at the
cursor position. To do this, one previously has to cut a block with the
:hp2.CTRL-b p:ehp2. command. The paste buffer is not emptied by this command.
Hence one can repeatedly insert the same block by iterating the command.
:li.:hp2.Left mouse button with moving:ehp2.
:lp.By pressing the left mouse button on the first block character and
releasing it on the first non-block character, one marks a 
:link reftype=hd res=15.normal block:elink..
:li.:hp2.Right mouse button with moving:ehp2.
:lp.By pressing the right mouse button on the first block character and
releasing it on the first non-block character, one marks a 
:link reftype=hd res=15.rectangular block:elink..
:li.:hp2.Clicking the right mouse button:ehp2.
:lp.has two functions depending on whether a block is marked in the
current window and whether a block is contained in the paste buffer.
If a block is marked in the current window, then the cut command is
performed, storing the block in the paste buffer. In addition the block
is unmarked.
:lp.If no block is marked in the current window, but the paste buffer is
not empty, then the paste buffer contents are inserted at the mouse position
(corresponds to the paste command). If the paste buffer is empty, only
the cursor is positioned at the mouse position.
:li.:hp2.Middle mouse button with left mouse button:ehp2.
:lp.If the middle mouse button is held down and then the left button is
pressed, a block marked in the current window is moved to the mouse position.
If no block was marked in the current window, nothing happens.
:li.:hp2.Middle mouse button with right mouse button:ehp2.
:lp.If the middle mouse button is held down and then the right button is
pressed, a block marked in the current window is deleted.
If no block was marked in the current window, nothing happens.
:eul.

.* ************************************
:h2.Window commands
.* ************************************

:p.The most obvious feature of :hp4.EHP:ehp4. is the possibility to
edit multiple texts quasi simultaneously by using windows. The editor
functions for the use of windows are explained below.

:h2 res=5.Window switching
:ul.
:li.:hp2.CTRL-w p:ehp2.
:lp.makes the window which was previously active become the current window.
Successively using :hp2.CTRL-w p:ehp2. thus jumps back and forth between two
windows.
:li.:hp2.CTRL-w n:ehp2.
:lp.makes the next window the current one (whatever 'next' will be). Only
one this about 'next' is asserted: one can use it to cycle through all
opened windows.
:li.:hp2.CTRL-w d:ehp2.
:lp.makes a window containing a certain file become the current window.
The file name must be entered in the 
:link reftype=hd res=12.status line:elink. after issuing the command.
If no file with the desired name is in any window, one gets the option to
load the specified file into a new window.
:li.:hp2.CTRL-w i:ehp2.
:lp.makes the window with a certain number the current window. The window
number is queried in the status line. If a window with the specified number
does not exist, an error message is displayed.
:li.:hp2.Left mouse button:ehp2.
:lp.By clicking with the left mouse button, the window on which the mouse
points becomes the current window and the cursor is placed on the mouse
position (except the window frame was hit).
:eul.

.* ************************************
:h3 res=16.Changing size and position
.* ************************************

:p.The size and position of each window can be changed either by using the
corresponding key commands or by using the mouse. In general it is advisable
to use the mouse. Key commands for that take some time to get used to...
:ul.
:li.:hp2.CTRL-w s:ehp2.
:lp.leads to a mode where you can use the cursor keys to resize the
current window. Only the corners of the window frame are displayed. Cursor
keys move the lower right corner. Hitting :hp2.ENTER:ehp2. confirms the
chosen size.
:lp.By pressing 's' during adjustment one can toggle the stepwidth between
5 and 1.
:li.:hp2.CTRL-w m:ehp2.
:lp.leads to a mode, in which the position of the current window can be
changed. The whole window, of which only the corners are displayed during
movement, can be moved via the cursor keys. As in resizing, pressing 's'
toggles the stepwidth between 5 and 1.
:li.:hp2.CTRL-w t:ehp2.
:lp.:hp4.EHP:ehp4. stores two positions for each window. When a window is
opened, both are set to full size. When you now move or resize the window,
the second stored size/position remains unchanged. You can switch between
these two sizes/positions by using this command. Of course the second
position remains only as long full screen as you don't change it. It is
thus possible to work with two size/position pairs which are both
non-fullscreen.
:li.:hp2.Left mouse button on lower right corner:ehp2.
:lp.By pressing the left mouse button on the lower right window corner
and holding
it down, one can resize the window by moving the mouse with the left button
pressed. As when changing window position by key commands, the window
size is indicated only by its four corners.
Releasing the button confirms the chosen size.
:li.:hp2.Left mouse button on upper window border:ehp2.
:lp.By pressing the left mouse button on the top window border and holding
it down, one can move the window by moving the mouse with the left button
pressed. As when changing window position by key commands, the window
position is indicated only by its four corners.
Releasing the button confirms the chosen position.
:li.:hp2.Left mouse button on upper right corner:ehp2.
:lp.By pressing the left mouse button on the upper right window corner
and holding it down, one can switch between the two window sizes. This 
corresponds to the key command :hp2.CTRL-w t:ehp2..
:eul.

.* ************************************
:h3.Open and close windows
.* ************************************

:p.:hp4.EHP:ehp4. allows to have as many windows open simultaneously, as 
there are 32-bit integers (though this limit may be somewhat theoretically, 
since memory limits will be much more severe.

:ul.
:li.:hp2.CTRL-w o:ehp2.
:lp.asks for the file name in the :link reftype=hd res=12.status line:elink.
and loads the named file into a new window. If the file is already in an
open window, :hp4.EHP:ehp4. asks, whether it is desired to switch to that
window instead of opening a new one.
:lp.The newly opened window is assigned the smallest free window number and
:link reftype=hd res=16.both window sizes:elink. are set to the size of the
screen. The cursor is placed in the first line and the first column
and autoindent is activated, if specified in :link reftype=hd res=2.EHPINIT:elink..
The :link reftype=hd res=9.tab-compression:elink. is activated by default.
The tab length is set to the default 8, if not specified different by command
or via :link reftype=hd res=2.EHPINIT:elink.. The window is put to insert
mode by default and underlining is turned off. 
:link reftype=hd res=8.Shell mode:elink. is deactivated by default.
:li.:hp2.CTRL-w c:ehp2.
:lp.closes the current window. If changes were made to the text (indicated
by :hp2.MODIFIED:ehp2. in the bottom frame border), a confirmation is
requested, avoiding accidental loss of changes.
:eul.

.* ************************************
:h2 res=6.Insert and delete
.* ************************************

:p.:hp4.EHP:ehp4. distinguishs the insert mode and the overwrite mode.
The mode is local to each window and is indicated in the top frame border
(:hp2.Insert:ehp2. or :hp2.Overwrt:ehp2.). If in insert mode, typed
characters are inserted in the text, in overwrite mode existing characters
are overwritten. In order to switch between modes, use the following:
:ul.
:li.:hp2.Ins:ehp2.
:lp.toggles between insert and overwrite mode
:li.:hp2.Del:ehp2.
:lp.deletes the current character (the one the cursor is on). If there are
characters to the right of the cursor, they are pulled left one position.
If the cursor was right of the end of the line, hitting :hp2.Del:ehp2. causes
the current line to be joined with the subsequent one (see also 
:hp2.CTRL-j:ehp2. described below).
:li.:hp2.Backspace:ehp2.
:lp.deletes the character left to the cursor. Characters to the right of
the cursor including the one the cursor is on are pulled left one position.
If the cursor was on the first column of a line, then the current line is
joined with the line above, provided that the current line was not the first
line of the current file. See also :hp2.CTRL-j:ehp2. described below for 
joining lines.
:li.:hp2.CTRL-t:ehp2.
:lp.deletes all characters beginning with the one under the cursor up to
the last character before the next word. If there is no next word, because
the cursor is placed in the last word of the line, then all characters up
to the end of the line are deleted.
:li.:hp2.CTRL-y:ehp2.
:lp.deletes the current line. It can be restored with :hp2.CTRL-k:ehp2. 
as described below.
:li.:hp2.CTRL-k:ehp2.
:lp.inserts the last deleted line before the current line.
:li.:hp2.CTRL-q:ehp2.
:lp.deletes from the cursor to the right up to the end of the line including
the current character.
:li.:hp2.CTRL-a:ehp2.
:lp.inserts an empty line at the current cursor position. The current line
is thus moved down one line. If :link reftype=hd res=9.autoindent:elink.
is active, then the cursor is indented accoring to the current line. That
means, if e.g. the current line starts with 5 blanks and :hp2.CTRL-a:ehp2.
is performed with autoindent active, then the cursor is positioned in column
6 of the inserted line. Note, that only non-underlined blanks count for
this purpose, as well as only non-underlined blanks are inserted for
doing the indentation, regardless of the current 
:link reftype=hd res=7.underline mode:elink..
:li.:hp2.CTRL-o:ehp2.
:lp.inserts a line below the current line. If autoindent is active, then
the cursor is indented according to the current line. That
means, if e.g. the current line starts with 5 blanks and :hp2.CTRL-a:ehp2.
is performed with autoindent active, then the cursor is positioned in column
6 of the inserted line. As with all indentation, underlined blanks do not
count as blanks for determining the indent width. Also, the inserted
blanks are not underlined, no matter what the current 
:link reftype=hd res=7.underline mode:elink. is.
:li.:hp2.ENTER:ehp2.
:lp.If in insert mode, the current line is splitted at the cursor position 
and the part from the cursor to the right is placed in the inserted line. 
The new line is indented according to the splitted line, if :link reftype=hd 
res=9.autoindent:elink. is active. For this indentation always non-underlined
blanks are used, for doing the indentation as well as for determining
the indentation width (see also :link reftype=hd res=7.Underline:elink.). 
The cursor will be located in the first column of the inserted line 
afterwards. If in overwrite mode, then the cursor is placed in the first 
column of the next line, if the current line is not the last line in the 
file. No line is inserted in overwrite mode.
:li.:hp2.CTRL-j:ehp2.
:lp.joins the current line with the subsequent line. First the cursor
is placed two characters behind the end of the current line. Then the
following line is moved to the cursor position, if the length of the
line doesn't exceed 500 characters (see 
:link reftype=hd res=13.Limits and Bugs:elink.). While doing this, leading
blanks are removed from the line moved to the corsor position. So, there
remains exactly one blank character between the current line and the
following line.
:lp.Note the funny feature, that CTRL-ENTER is equal to CTRL-j under OS/2 and
thus invokes the joining of lines.
:eul.

.* ************************************
:h2 res=17.Find and replace
.* ************************************

:p.Searching with :hp4.EHP:ehp4. supports regular expressions for matching
patterns. All expressions available with :hp2.grep:ehp2. should work with
:hp4.EHP:ehp4., too. If there is much searching done on special characters
like '[' or '.', then regular expressions might slow down work and can 
therefore be turned off by using
:link reftype=hd res=2.EHPINIT:elink. or by command (see 
:link reftype=hd res=14.Miscellaneous:elink. for a description of the 
command) or by option when doing the search.

.* ************************************
:h3 res=18.Regular expressions
.* ************************************

:p.Regular expression pattern matching is done with the 
:hp2.GNU REGEX:ehp2. package. Thus the recognized expressions should be the 
same as with :hp2.grep:ehp2.. The most important regular expression rules
will now be summarized. Note, that 'X' stands for an arbitrary regular
expression.

:dl break=fit.
:dthd.:hp5.Pattern:ehp5.
:ddhd.:hp5.matches:ehp5.
:dt.non-special character
:dd.Each non-special character builds a basic regular expression.
:dt.:hp2..:ehp2. (dot) 
:dd.matches any character.
:dt.:hp2.\(X\):ehp2.
:dd.groups the expression X. May be used nested.
:dt.:hp2.\n:ehp2. 
:dd.where n is a number, refers to the n-th group.
Groups are numbered in increasing order depending on the position
of the left group parenthesis from left to right. The \n refers to the
text matched by the group (not the pattern itself). That means for
example, that '\(oh+\) \1' matches 'oh oh' and 'ohh ohh', but not
'oh ohh' or 'ohh oh'.
:dt.:hp2.X+:ehp2. 
:dd.matches an non-zero number of subsequent X's.
:dt.:hp2.X*:ehp2. 
:dd.matches an arbitrary number of subsequent X's.
Zero occurrences are included. For example '\(ab\)*' matches 'abab'.
:dt.:hp2.X?:ehp2. 
:dd.matches zero or one occurrence of X.
:dt.:hp2.[Y]:ehp2. 
:dd.matches all characters contained in Y.
Here Y is not an arbitrary regular expression. Instead, Y may consist of
a number of single characters ([yYnN]) and of sequences. Sequences are
specified by the start and the end of the sequence separated by a dash.
For example [a-zA-Z] matches all letters, and [0-9] matches all digits.
:dt.:hp2.^X:ehp2. 
:dd.matches X, if it is found at the beginning of the line.
:dt.:hp2.X$:ehp2. 
:dd.matches X, if it is found at the end of the line.
:dt.:hp2.\<X:ehp2. 
:dd.matches X at the beginning of a word.
:dt.:hp2.X\>:ehp2. 
:dd.matches X at the end of a word.
:dt.:hp2.\:ehp2. 
:dd.is the escape character.
To avoid interpretation of a special character (like '[' or '*'),
such characters must be preceded by a backslash. To search for a backslash
double the backslash ('\\'). Note, that a parenthesis matches itself, while
an escaped parenthesis is good for grouping patterns (see above).
:edl.

.* ************************************
:h3 res=22.Find and replace commands
.* ************************************

:ul.
:li.:hp2.CTRL-f f:ehp2.
:lp.searches for a pattern, which is asked for in the 
:link reftype=hd res=12.status line:elink.. By default the search is
case sensitive and is directed forward. It starts at the character right to
the cursor (important for repeated searches) and wraps around the end of
the text to the beginning. By default, the search pattern is interpreted as
a :link reftype=hd res=18.regular expression:elink. as used with 
:hp2.grep:ehp2.. See above for
:link reftype=hd res=17.turning regular expressions off:elink. and note
the options 'x' and 'n' explained in the sequel. When the search pattern
is empty, no search is done.
:lp.When the search pattern has been entered, :hp4.EHP:ehp4. asks for
the options in the status line. The following options can be combined as you
like (no options is allowed, too; just press :hp2.ENTER:ehp2.). 
Exceptions are, that :hp2.b:ehp2. and :hp2.e:ehp2. exclude each other and 
that :hp2.r:ehp2. has no effect, if :hp2.b:ehp2. or :hp2.e:ehp2. are used. 
Available are:
:dl compact tsize=15.
:dthd.:hp5.Option:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.:hp2.<number>:ehp2. 
:dd.find the <number>th occurrence of the pattern
:dt.:hp2.r:ehp2. 
:dd.search in reverse direction
:dt.:hp2.i:ehp2.
:dd.ignore case (case insensitive search)
:dt.:hp2.w:ehp2.
:dd.find words only (found pattern must be limited by a
:link reftype=hd res=19.word separator:elink.
:dt.:hp2.b:ehp2.
:dd.search from the beginning of the text
:dt.:hp2.e:ehp2.
:dd.search from the end of the text in reverse direction
:dt.:hp2.x:ehp2.
:dd.interpret regular expressions in search pattern
:dt.:hp2.n:ehp2.
:dd.don't interpret regular expressions in search pattern
:edl.
:li.:hp2.CTRL-f r:ehp2.
:lp.ssearches a specified number of times for a pattern and replaces it.
By default, :hp4.EHP:ehp4. asks before replacing a found pattern. This can
be turned off using the options. Another default is, to underline the replace
expression according to the underlining of the first character of the found
pattern. Imagine, the text was 'documentation for editor :hp5.EHP:ehp5. in
INF format', and the replace is started with search pattern 'EHP' and
replace pattern 'VI', then the replacement string is underlined, since the
first letter of the found 'EHP' was underlined. The resulting text is thus
'documentation for editor :hp5.VI:ehp5. in INF format'. The underlining of
the replace expression can be modified using the options.
Note the difference between the number option with the find and the replace
command: when doing find with option '10', the 10th occurrence is found.
When replacing with option '10', the first 10 occurrences are replaced
(NOT only the 10th occurrence!).
Options are as with find, additionally the following options are available:
:dl compact.
:dthd.:hp5.Option:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.:hp2.a:ehp2.
:dd.replace all occurrences in the current file
:dt.:hp2.g:ehp2.
:dd.global: don't ask for replacement
:dt.:hp2.u:ehp2.
:dd.underline according to underline state
This means, that the replace expression underlining is not determined
from the underline state of the found pattern's first character, but from
the current underline state in the current window (indicated in the window's
top border). Thus, if underlining is turned on for the current window, then
the replace expression is underlined regardless of underlining of the found
pattern.
:edl.
:li.:hp2.CTRL-n:ehp2. or :hp2.CTRL-f a:ehp2.
:lp.repeats the last search or replace action. The search and replace 
patterns remain the same. Only the number provided in the options is
reset to 1 (nevertheless, when doing replace, option a is interpreted as 
before), and options b and e are removed.
:eul.

.* ************************************
:h3.Find multiple spaces
.* ************************************

:p.When searching or replacing, the search pattern may contain blanks.
Due to :hp4.EHP:ehp4.'s internal representation of text, blanks beyond the
end of the line are not matched. That implies, that an empty line is not
matched by a search pattern consisting of one blank. Note this for example,
if a search is done for 'not ': a 'not' at the end of line is not found
with this pattern. Use either the w option (for finding words only) or
construct a suitable :link reftype=hd res=18.regular expression:elink..

.* ************************************
:h2 res=9.Tabs and indentation
.* ************************************

:p.:hp4.EHP:ehp4. supports two kinds of tabstops: tabs with fixed width
and tabs depending on word boundaries of the above line. Which kind of tabs
is used depends on the autoindent mode of the current window.

:p.Tabs are expanded to spaces in the text. If tab-compression is activated,
each 8 (independent of the chosen tabwidth) leading spaces on a line are 
compressed into a tab when the text
is written to a file. Otherwise they remain spaces. Tab-compression is
turned on by default.
:ul.
:li.:hp2.CTRL-s a:ehp2.
:lp.toggles the autoindent mode for the current window. The mode is displayed
in the top window border (:hp2.Insert:ehp2. or :hp2.Overwrt:ehp2.).
:eul.

.* ************************************
:h3.With autoindent
.* ************************************

:p.If autoindent is active for the current window, the tabwidth is 
determined by looking at the line above the current line. The tab forward 
skips as far as a 'word right' command would skip in the above line. 
Similar, a tab back skips as far as a 'word left' command would skip in the 
above line. This is good for indenting comments or for jumping to the same 
indentation as used in the above line. If the cursor is located beyond the 
end of the above line, tabs proceed as without autoindent. Note, that tab 
backward and tab forward are inverse operations except when the cursor is 
located below the last word of the above line. In this case, the tab forward 
skips below the position immediately behind the end of the word in the above 
line, while the tab backward skips below the beginning of that word.

:p.If a line is splitted in autoindent mode using :hp2.ENTER:ehp2.,
the line on which the cursor is after the operation is indented that way,
that it starts below the start of the first word of the above line.
Inserting a line before or after the current line using :hp2.CTRL-a:ehp2. 
or :hp2.CTRL-o:ehp2. places the cursor in the column, where the first word
of the previously current line starts.

.* ************************************
:h3.Without autoindent
.* ************************************

:p.In this mode the tabwidth is determined only by the predefined tabwidth, 
which is 8 by default. This tabwidth can be modified by two ways: These are 
the environment variable :link reftype=hd res=2.EHPINIT:elink. or by the 
command

:ul.
:li.:hp2.CTRL-s t:ehp2.
:lp.asks for the new tabwidth, which can be entered in the 
:link reftype=hd res=12.status line:elink.. As default for this input
yields the currently chosen tabwidth. Thus, pressing :hp2.ENTER:ehp2.
upon this query keeps the tabwidth unchanged.
:eul.

.* ************************************
:h2 res=26.Repeating
.* ************************************

:ul.
:li.:hp2.CTRL-r:ehp2.
:lp.repeats a command to specify a certain number of times. :hp4.EHP:ehp4.
first asks for the number of repetitions in the 
:link reftype=hd res=12.status line:elink.. Here an empty or zero input
causes the command to repeat nothing. If a non-zero input was made, the
command to repeat is asked for. One can enter the command as if it should
be executed directly with one exception: the :hp2.ENTER:ehp2. key must
be escaped by prefixing it with :hp2.CTRL-c:ehp2. (see also 
:link reftype=hd res=20.Controlcodes:elink.), since pressing the 
:hp2.ENTER:ehp2.-key unescaped will be interpreted as the end of the
command sequence. Note, that during input of the command the usual help texts will be
displayed in the status line, if not turned off.
:lp.Repeating may be nested up to a depth of 20. Note, that 
:link reftype=hd res=21.macros:elink. also each take one of those nesting
levels. So, if your command to repeat contains a macro, which itself
has a nesting depth of say :hp1.n:ehp1., then there must be less than
20-:hp1.n:ehp1. nested repeats around it.
:lp.Look at the following example for a better understanding. It causes
five new lines to be inserted at the cursor position, where each of them
is filled with the string 'hello':
:dl compact tsize=18.
:dthd.:hp5.Key commands:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.:hp2.CTRL-r:ehp2.
:dd.For invoking the repeat command
:dt.:hp2.5 ENTER:ehp2.
:dd.For specifying the number of repetitions
:dt.:hp2.CTRL-c:ehp2.
:dd.For escaping the subsequent :hp2.ENTER:ehp2.
:dt.:hp2.ENTER:ehp2.
:dd.For inserting the new line at the cursor position
:dt.:hp2.hello:ehp2.
:dd.The text to be inserted in the new line
:dt.:hp2.ENTER:ehp2.
:dd.For terminating the command sequence to repeat
:edl.
:li.:hp2.CTRL-n:ehp2. or :hp2.CTRL-f a:ehp2.
:lp.repeats the last :link reftype=hd res=22.find/replace:elink. command.
:eul.

.* ************************************
:h2 res=7.Underline
.* ************************************

:p.:hp4.EHP:ehp4. offers the possibility to enter underlined text. See
:link reftype=hd res=23.Fileformat:elink. for the representation of
underlined characters in the resulting file and consequences of this
representation. On the screen, underlined
characters are displayed in :link reftype=hd res=24.red color:elink..
The underline mode is local to each window and is displayed in the top
window border (:hp2.Under:ehp2. in activated underline mode, nothing
in non-active underline mode).

:p.If underlining is active, each inserted / overwritten character is
automatically underlined. Furthermore, underline mode influences 
:link reftype=hd res=22.replacing:elink.

:p.To toggle the underline mode for the current window, use the following
command:
:ul.
:li.:hp2.CTRL-u:ehp2.
:lp.toggles the underline mode for the current window.
:eul.

.* ************************************
:h2.Automatic line break
.* ************************************

:p.:hp4.EHP:ehp4. supports a mode for automatically breaking the current 
line if the cursor otherwise would leave the window to the right. If this 
linebreak mode is active, then if the cursor would step on the right window 
margin, the next blank left to the cursor is searched, the cursor is placed 
right to this position (i.e. on the beginning of the current word) and a 
newline is executed.

:p.Note, that if the :link reftype=hd res=9.autoindent mode:elink. is active,
then the splitting of the current line will indent the new created line 
according to the current line. If :link reftype=hd
res=6.overwrite mode:elink. is active, then the cursor will be placed on the
next line by the execution of the newline. This is exactly what happens in 
overwrite mode, if the :hp2.ENTER:ehp2. key is hit. This, of course, does not
make much sense. So the use of :link reftype=hd res=6.insert mode:elink. is 
highly recommended when using the automatic linebreak mode.

The automatic linebreak mode is toggled by the following command:
:ul.
:li.:hp2.CTRL-s l:ehp2.
:lp.toggles the automatic linebreak mode
:eul.

It can furthermore be set as the default for window creation by the flag 
:hp2.linebrk:ehp2. in the environment variable :link reftype=hd
res=2.EHPINIT:elink..

.* ************************************
:h2.Reflowing of paragraphs
.* ************************************

:p.:hp4.EHP:ehp4. enables its user to reflow paragraphs so that they
are as close as possible aligned to the right window margin. Reflowing is
started at the corsor line and ends before the next empty line or the end
of the text.

:p.The margins used for reflowing are as follows: As right margin the
right window margin is used. So if reflowing to a certain size differing
from the current window size is desired, first resize the window, then
reflow and afterwards change the window size back to the previous size (see
also :link reftype=hd res=16.changing window size:elink.). What is taken
to be the left margin for reflowing depends on the 
:link reftype=hd res=9.autoindent:elink. flag. If it is set, then the
column of the first non-whitespace in each current line is taken as the
left reflow margin. Otherwise, the left reflow margin defaults to column 1.

:p.Reflowing only breaks lines at blank characters. Character sequences
separated only by the so-called 'word separators' (e.g. '-' or '.') are not 
broken apart. The cursor is positioned at the character where it was when
reflowing started. This may of course be in a line further down and more to
the left due to moving of that character during the reflowing process.

:p.Reflowing might not work for two reasons. First, the maximum
:link reftype=hd res=13.line capacity:elink. of :hp4.EHP:ehp4. might become 
exceeded by the breaking of the lines. Second, an error may occur, when
a line is shorter than the reflow width and the subsequent line can't be
joined because the length would go beyond :hp4.EHP:ehp4.'s 
:link reftype=hd res=13.line length limit:elink..

:ul.
:li.:hp2.CTRL-d:ehp2.
:lp.initiates the reflowing of the current paragraph beginning at the
cursor position and ending at the next empty line or the end of the text.
:eul.

:p.Handle this command with care, since at the moment there exists no
undoing for this command. So, if the empty line you expected to be there 
isn't there, then the results may be quite farther-going than you wanted.
You can stop this command by hitting :hp2.CTRL-Break:ehp2..

:p.Regrettably, there was no mnemomical key binding available for this
command. If someone has a good idea, how :hp2.CTRL-d:ehp2 could be
seen as an mnemonic for 'reflowing a paragraph', please report!

.* ************************************
:h2 res=20.Controlcodes
.* ************************************

:p.Controlcodes cannot simply be typed to insert them into the text beacuse
they would issue a command. Nevertheless :hp4.EHP:ehp4. allows to insert
them by escaping them with :hp2.CTRL-c:ehp2..

:ul.
:li.:hp2.CTRL-c:ehp2. <character>
:lp.insert <character> into the text, regardless whether it is a controlcode
or even the escape key. Only a NULL character cannot be inserted at all,
since this would mark the end of line.
:hp2.CTRL-c CTRL-c:ehp2. inserts a ^C into the text,
for example. Characters with ASCII code smaller than 28 are displayed as
invers character with 64 added to the ASCII code. Thus, e.g. an 
:hp2.CTRL-a:ehp2. is displayed as invers A.
:eul.

.* ************************************
:h2 res=10.Quit EHP
.* ************************************

:ul.
:li.:hp2.CTRL-x s:ehp2.
:lp.leaves :hp4.EHP:ehp4. and saves all modified files. If the 
:link reftype=hd res=14.backupoption:elink. is active, then the old file
is saved by appending '.bak' to its name. The file attributes remain as
they were when loading the file, they are set to writeable, non-system and
non-hidden, if a new file is saved. If files cannot be saved for some 
reasons, a warning is displayed. :hp4.EHP:ehp4. offers the possibility
to leave anyway or to proceed with further editing.
:li.:hp2.CTRL-x n:ehp2.
:lp.leaves :hp4.EHP:ehp4. without saving modified files. For safety, this
action has to be acknowledged in the 
:link reftype=hd res=12.status line:elink..
:li.:hp2.CTRL-x S:ehp2.
:lp.leaves :hp4.EHP:ehp4. without saving modified files after an
acknowledgement, additionally the :link reftype=hd res=25.load file:elink. 
is executed.
:li.:hp2.CTRL-x N:ehp2.
:lp.leaves :hp4.EHP:ehp4. with saving modified files after an
acknowledgement (see above), additionally the 
:link reftype=hd res=25.load file:elink. is executed.
:eul.

.* ************************************
:h2.Undo
.* ************************************

:p.The only undo implemented so far is restoring a deleted line.
:ul.
:li.:hp2.CTRL-k:ehp2.
:lp.inserts the last deleted line before the current line and makes this
line the current line. This command can be executed multiply (good for
quickly duplicating the current line or copying it to many different
locations).
:eul.

.* ************************************
:h2 res=21.Macros
.* ************************************

:p.Macros are user-defined key command sequences (no mouse commands can
be recorded within a macro). They can be recorded,
executed, nested and repeated. Note, that the maximum nesting depth
is limited to 20.

:ul.
:li.:hp2.CTRL-v X:ehp2. (where X stands for an upcase letter)
:lp.records macro X. One can now input the key sequence, which is to
make up the macro. The maximum macro length is limited to 500 characters
(the maximum line length), where keys producing more than one character
are counted as multiple characters. If this sould be a limit to you, make use
of nested macros and the :link reftype=hd res=26.repeat command:elink..
During input of the macro, :hp4.EHP:ehp4. displays the usual helptexts in
the :link reftype=hd res=12.status line:elink., if they are not
disabled. To finish the macro recording hit the :hp2.ENTER:ehp2. key.
To insert an :hp2.ENTER:ehp2. into the macro, use the
:link reftype=hd res=20.controlcode:elink. technique, that means, hit
:hp2.CTRL-c ENTER:ehp2..
:li.:hp2.CTRL-v x:ehp2. (where x stands for an lowercase letter)
:lp.executes macro x. Note that this command cannot be entered when
an input in the status line is expected (e.g. the search expression).
If this is desired, the command issuing the status line query has to
be included in the macro. If the chosen macro is empty, a warning is
displayed. For stopping the execution of a macro see
:link reftype=hd res=27.aboring running macros:elink..
:eul.

.* ************************************
:h2 res=27.Abort running macro / repeat function
.* ************************************

:p.Running :link reftype=hd res=21.macros:elink. and
running :link reftype=hd res=26.repeat commands:elink. can be aborted.
:ul.
:li.:hp2.CTRL-Break:ehp2.
:lp.aborts all running macros and repeat commands. That means, that also
in case of a running nested macro all involved macros are terminated.
:eul.

.* ************************************
:h2 res=8.Shell escape
.* ************************************

:p.:hp4.EHP:ehp4. allows to execute shell commands from within a text
window. This can be done by using the shell mode.
:ul.
:li.:hp2.CTRL-s s:ehp2.
:lp.activates/deactivates the shell mode for the current window.
If the shell mode is active for the current window can be determined by
looking at the top border of the current windows's frame. If
:hp2.SHELL:ehp2. is displayed there, then shell mode is active.
To issue a command, hit :hp2.ENTER:ehp2.. Then the part of the current line
to the left of the cursor will be taken as the command and passed to the
shell. The standard output will be inserted as a block at the cursor
position. Note that the current line will be split at the point where
the cursor was when hitting :hp2.ENTER:ehp2..
:lp.Commands expecting further user input should not be executed in this
way, since one cannot provide any further interactive input after issuing
the command. The result of invoking such a program would be, that the
shell didn't return. In this case the only way to get back to :hp4.EHP:ehp4.
is to hit :hp2.CTRL-Break:ehp2., which must not be catched by the running
program.
:lp.Now, which shell is executed? :hp4.EHP:ehp4. always uses the shell
specified by the environment variable :hp2.COMSPEC:ehp2.. This is
usually c&colon.\os2\cmd.exe. If the command execution caused an error, a
corresponding message is displayed.
:eul.


.* ************************************
:h2 res=14.Miscellaneous
.* ************************************

:ul.
:li.:hp2.CTRL-s a:ehp2.
:lp.toggles the autoindent mode for the current window. If autoindent is
active for the current window, this is indicated by the string
:hp2.Indent:ehp2. in the top frame border.
:li.:hp2.CTRL-s t:ehp2.
:lp.sets the tablength for the current window. The new tablength must
be entered in the :link reftype=hd res=12.status line:elink.. As default
serves the current tablength, which means, that pressing :hp2.ENTER:ehp2.
on the query leaves the tablength for the current window unchanged.
Thus this command can also be used to determine the current window's
tablength. See also :link reftype=hd res=9.Tabs and indent:elink. for
more detailed information about tabs.
:li.:hp2.CTRL-s h:ehp2.
:lp.toggles activation of helptexts. If active, :hp4.EHP:ehp4. displays
a helptext in the status line after the first key of a multi-key command.
If the machine or the display or whatever is too slow or one just wants
the helptexts to disappear, then helptexts can be deactivated by using
this command.
:li.:hp2.CTRL-s b:ehp2.
:lp.toggles the blockhighlighting. By default, a marked block is
displayed highlighted. This takes some time. As for helptexts, if this
seems to be too slow, this highlighting can be deactivated using this 
command.
:li.:hp2.CTRL-s c:ehp2.
:lp.toggles the compression of tabs for the current window (see also
:link reftype=hd res=9.Tabs and indent:elink.). If activated, this is
displayed by the string :hp2.Tabs:ehp2. in the top frame border, otherwise
:hp2.Spcs:ehp2. is displayed indicating, that leading spaces are written
out as spaces and not compress.
:li.:hp2.CTRL-s r:ehp2.
:lp.Toggles the default for the usage of regular expressions in searching.
This default overrides the default set in 
:link reftype=hd res=2.EHPINIT:elink. but can be overridden by a search
option (see also :link reftype=hd res=22.Find and replace:elink.).
:li.:hp2.CTRL-s .:ehp2.
:lp.toggles the creation of backup (.bak) files.
:eul.

.* **********************************************************************
:h1 res=4.Configuration file
.* **********************************************************************

:p.The configuration file is updated upon leaving :hp4.EHP:ehp4..
It contains global flags (like blockhighlight, creation of backup files
etc.), all defined macros, information about window size, position and
files loaded, and the position of all defined markers. Thus is allows to
restore the state exactly as it was when reinvoking :hp4.EHP:ehp4..

:p.When starting :hp4.EHP:ehp4. the configuration file is looked for.
If it is found, the global flags and the macros are restored in each case.
If no filenames were specified in the command line, :hp4.EHP:ehp4. loads
the files specified in the configuration file into windows with the
same size and position they had when leaving :hp4.EHP:ehp4. the last time.
In this case the markers are taken over, too.

:p.The name of the configuration file is :hp2.config.ehp:ehp2. in the
current directory by default. This is the name, under which it is
written to disk as well as the name under which it is searched when
starting :hp4.EHP:ehp4.. This name can be modified by an entry in the
environment variable :link reftype=hd res=2.EHPINIT:elink.. Via this
option even an absolute path for the configuration file may be specified
with the effect of having all ever defined macros global, for example.


.* **********************************************************************
:h1 res=2.Environment variable EHPINIT
.* **********************************************************************

:p.If an environment variable named :hp2.EHPINIT:ehp2. is provided,
:hp4.EHP:ehp4. reads its contents before reading the 
:link reftype=hd res=4.configuration file:elink. (so defaults specified
in :hp2.EHPINIT:ehp2. may be overridden by values stored in the
configuration file). Set :hp2.EHPINIT:ehp2. by invoking

:p.:hp2.SET EHPINIT=...:ehp2.

:p.from within config.sys or from the command line,
where ... is an arbitrary concatenation of the following options:
:dl compact tsize=20 break=fit.
:dthd.:hp5.Option:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.autoind
:dd.Autoindent mode will be active when opening a new window.
:dt.nohelp
:dd.Turns off helptexts when :hp4.EHP:ehp4. is invoked.
:dt.noshowblock
:dd.Blockhighlighting is turned off by default.
:dt.nobak
:dd.When saving a file, no .bak file will be generated, if a file with
this name already exists.
:dt.deftab <n>
:dd.where <n> stands for a number. This sets the default tablength for
a new window to <n>. <n> must be between 0 and 500.
:dt.conffile <f>
:dd.where <f> is a filename path (absolute or relative). This states, that
the :link reftype=hd res=4.configuration file:elink. is stored and loaded
with the name <f>. An example might be 
:hp2.conffile c&colon.\config.ehp:ehp2..
:dt.loadfile <f>
:dd.The :link reftype=hd res=25.load file:elink. is searched under path
<f>.
:dt.screen <mode>
:dd.where mode is <#columns>x<#lines> (instead of x also X is possible).
This sets the screen size to the desired values. The provided values must
be supported by the video card and the operating system, otherwise
the values default to 80x25, which should always be possible. An example
might be :hp2.screen 80x50:ehp2.. Note, that after leaving :hp4.EHP:ehp4.,
the screen size is switched back to the state it was in before invoking
:hp4.EHP:ehp4..
:dt.keys <f>
:dd.Tells :hp4.EHP:ehp4. to load the keyboard mapping from file <f> instead
of :hp2.tasten.inf:ehp2. in the current directory. <f> may contain an
absolute path.
:dt.noregex
:dd.Turns off regular expressions for searching be default. This can be
overridden by setting a global flag within :hp4.EHP:ehp4. by a command
or by using a :link reftype=hd res=17.search option:elink..
:dt.linebrk
:dd.Turns on the automatic linebreak mode by default. It is overruled
by the manual setting for a window as well as by the values stored for an 
open window in the :link reftype=hd res=4.config file:elink..
:dt.clr_text <fg_color+16*bg_color>
:dd.sets the color of the text display. Note, that the frames around inactive
windows are drawn with the same color.
:dt.clr_frame <fg_color+16*bg_color>
:dd.sets the color of the frame for the active window. This is also the color
used for highlighting the mnemonical letters in the helpline.
:dt.clr_under <fg_color+16*bg_color>
:dd.sets the color used for displaying underlined characters
:dt.clr_block <fg_color+16*bg_color>
:dd.sets the color used for displaying a highlighted block.
:edl.

:p.The valued for :hp2.fg_color:ehp2. and :hp2.bg_color:ehp2. must lie
between 0 and 15 (inclusive). The bit values of each of them is defined as
follows:

:dl.
:dt.1
:dd.blue
:dt.2
:dd.green
:dt.4
:dd.red
:dt.8
:dd.light
:edl.

:p.Mixing the color bits yields the color resulting from the standard color
addition. Perhaps on the fullscreen display a light background may be
interpreted as blinking background. Check this out, if you want to use the
light-bit for your background setting and at the same time use :hp4.EHP:ehp4.
in a fullscreen session.

.* **********************************************************************
:h1 res=25.Load file
.* **********************************************************************

:p.When :link reftype=hd res=10.leaving :hp4.EHP:ehp4.:elink., this can be
done by a command, which invokes a batch file after terminating 
:hp4.EHP:ehp4.. By default, this batch file is searched in the current
directory and is assumed to be named :hp2.load.cmd:ehp2.. This default
name can be overridden using the environment variable 
:link reftype=hd res=2.EHPINIT:elink.. A possible application for this is,
that load.cmd contains the call of :hp2.make:ehp2. or a different
compilation tool, which performs some actions on the edited file(s).

.* **********************************************************************
:h1 res=30.Keyboard remapping (KM utility)
.* **********************************************************************

:p.To make :hp4.EHP:ehp4. suit individual keyboard layout requirements,
:hp4.EHP:ehp4. comes with a little tools enabling the modification and
creation of own keaboard layout files. Since this tool is used at most
only a few times, it is not very gold plated (not to say it is
rather prehistoric, but it satisfies the basic needs). This tool is called
:hp4.KM:ehp4., which stands for 'keymake'.

:p.:hp4.KM:ehp4. can in addition to the modification of keystrokes modify
the helptexts, that can be displayed after the first key of the command
in the status line. :hp4.KM:ehp4. assumes the keyboard file, which is
called :hp2.tasten.inf:ehp2., to be located in the current directory.
If no such file exists in the current directory, then an empty
one it is created.

.* *****************************
:h2.Main menu
.* *****************************

:p.After invoking :hp4.KM:ehp4., it is tried to read in the 
:hp2.tasten.inf:ehp2. file. If the file has the wrong format, this
is announced by an error message and the program stops.
Otherwise, a menu comprising four items will be displayed. These items
are

:ul.
:li.Change key combinations
:li.Change helptexts
:li.Show command names
:li.Exit (save keyboard file)
:eul.

:p.Issue the wished alternative by hitting the corresponding number (without
subsequent :hp2.ENTER:ehp2. key).

.* *****************************
:h2.Change keyboard command
.* *****************************

:p.This command allows to provide key combinations for one or more commands.
First the number of the command to change is queried. To answer this
question, the below described option 
:link reftype=hd res=28.Show command names:elink. is helpful.
Inputting the combination number is quitted by hitting :hp2.ENTER:ehp2..
If -1 is entered as key combination number, changing of combinations
is finished and :hp4.KM:ehp4. returns to the main menu.

:p.If a correct number was provided, :hp4.KM:ehp4. shows the current
mapping in decimal form. If e.g. the combination is currently
:hp2.CTRL-l:ehp2., then :hp2.12:ehp2. is displayed, since the ASCII
code for :hp2.CTRL-l:ehp2. is 12.

:p.If the provided combination number exceeded the number of currently
defined combinations, then a new command is established with a number
greater by one than the combination number with the previously highest 
number. This of course only makes sense, if a new :hp2.tasten.inf:ehp2.
file is created or indeed :hp4.EHP:ehp4. was extended by one or more
commands (this would need a source code change as well in :hp4.EHP:ehp4.
as in :hp4.KM:ehp4.). In the latter case the modification of 
:hp2.tasten.inf:ehp2. would probably have been done by the programmer
himself.

:p.Now :hp4.KM:ehp4. asks for the new key combination. This may consist
of any key combination not containing '/', since this is the key indicating
the end of the combination. It cannot be escaped. Note, that of course
events like 'hitting the left space key' or something like this can not
be input at this point. When chosing your combinations, note that commands
must not be prefixes of each other. This implies, that it is unconvenient
to use normal letters like a,b,c as the first character of a command,
since then inputting these characters in a text would not be easy anymore.

:p.The assertion, that commands must not be prefixes of each other is
checked after inputting the combination.  If a collision is detected,
this is announced by an error message and the old combination remains
unchanged. In case the entered combination was a new one (would have been
added as the last combination with the highest number), if a collision
is detected, no new combination is created.

:p.If the provided combination consists of more than one ASCII code
(note that this is also true for some function and cursor keys),
all existing helptexts are displayed together with their indices. 
:hp4.KM:ehp4. asks for a helptext index to link to the current combination.
This helptext would then be displayed in :hp4.EHP:ehp4.'s 
:link reftype=hd res=12.status line:elink.
after hitting the first character of the combination. If no helptext should
be displayed, then one has to enter -1 as helptext index (this makes sense
e.g. for the mentioned cases function keys or cursor keys).
If a non-existing helptext index is input, then :hp4.KM:ehp4. asks
for a new helptext. See :link reftype=hd res=29.Change helptext:elink. for 
detailed information about changing and inputting the helptexts.

.* *****************************
:h2 res=29.Change helptext
.* *****************************

:p.When this option is chosen, first all existing helptexts are displayed
together with their indices. :hp4.KM:ehp4. then asks for the number
of the helptext to change. If a non-existing number is entered, the
query is repeated.

:p.If the helptexts with the provided number exists, it is displayed
once more and the new helptext is asked for. To enter letters, that
should be highlighted in :hp4.EHP:ehp4.'s helptext display, such letters
must be enclosed in parenthesis. For example '(L)ist' causes 
':hp2.L:ehp2.ist' to be displayed. It is convention, to highlight the
letters, which invoke further commands.

:p.One terminates the helptext by pressing :hp2.ENTER:ehp2.. :hp4.KM:ehp4.
continues asking for helptext numbers, until -1 is entered. This returns
to the main menu.

.* *****************************
:h2 res=28.Show command names
.* *****************************

:p.All available editor commands are listed together with their numbers
and names. Using this command makes sense to get out the number of
a certain command, for which one wants to change the key combination.

:p.One screen page is filled with command names, then a message is
displayed, that a keypress is expected for the continuation of the listing.
Here, pressing :hp2.a:ehp2. aborts the listing. When the listing is
finished, one further keypressed is asked for, before :hp4.KM:ehp4.
returns to the main menu.

.* *****************************
:h2.Quit KM
.* *****************************

:p.This choice terminates the :hp4.KM:ehp4. program. The keyboard
mapping is written to the file :hp2.tasten.inf:ehp2. in the current
directory. A possibly existing file will be overwritten. Thus, save
one working keyboard mapping file to avoid accidental lost of keyboard
definitions. To suppress the writing of the keyboard mapping, terminate
the program by hitting :hp2.CTRL-Break:ehp2. from the main menu.
Note that all changes of this :hp4.KM:ehp4. session will then be lost!

.* *********************************************************************
:h1.Appendix
.* *********************************************************************

.* *****************************
:h2 res=13.Limits and bugs
.* *****************************

:p.The line length is currently limeted to 500. If this should be a problem
for anyone, please report (e.g. via email to auhl@fzi.de).
Each window may contain 2^31-1 lines. There must be no more than
2^31-1 windows opened concurrently.

:p.If the text in a window contains the mamimal number of lines, no blocks
can be moved anymore.

:p.An underscore - :hp2.CTRL-h:ehp2. combination ar the end of line is 
interpreted as underlined space.

:p.Tabs are not stored as tabs but expanded to spaces except at the
beginning of a line, where by default each 8 spaces are 
:link reftype=hd res=9.compressed to one tab:elink.. The latter option
allows the creation of makefiles, where the tab at the beginning
of a line has some concrete semantic. Still, this might be a limitation,
especially, if tables with constant column positions (almost) independent of 
the column contents should be created (like it is possible with the way,
VI handles tabs). Also, this might be confusing, if a text containing tabs
is loaded, modified and saved, because in this case the tabs in the middle
of a line are converted to blanks.

.* *****************************
:h2 res=1.Command reference
.* *****************************

:dl compact tsize=20 break=fit.
:dthd.:hp5.Key combination:ehp5.
:ddhd.:hp5.Associated command:ehp5.
:dt.CTRL-a                  
:dd.Insert line before current line
:dt.CTRL-b b             
:dd.mark beginning of block
:dt.CTRL-b c             
:dd.copy block to cursor position
:dt.CTRL-b d             
:dd.delete block
:dt.CTRL-b e             
:dd.end of rectangular block (good for moving comments)
:dt.CTRL-b f             
:dd.filter the block (e.g. pass the block through SORT)
:dt.CTRL-b i             
:dd.indent block (left or right)
:dt.CTRL-b m             
:dd.move block to cursor position
:dt.CTRL-b n             
:dd.end of normal block (with line wrap)
:dt.CTRL-b p             
:dd.copy contents of paste-buffer to cursor position
:dt.CTRL-b r             
:dd.read block from file and insert at cursor position
:dt.CTRL-b t             
:dd.cuT block (means: copy it to the paste-buffer)
:dt.CTRL-b u             
:dd.unmark block
:dt.CTRL-b w             
:dd.write block to file
:dt.CTRL-c                  
:dd.abort repeat function or macro execution,
insert control code in text or command
:dt.CTRL-d
:dd.Reflows the current paragraph
:dt.CTRL-e l             
:dd.load file in current window
:dt.CTRL-e m
:dd.save all modified files
:dt.CTRL-e r             
:dd.rename file
:dt.CTRL-e s             
:dd.save file contained in current window
:dt.CTRL-f a             
:dd.Again (repeat last find or replace)
:dt.CTRL-f f             
:dd.find. For options see :link reftype=hd res=17.Find and replace:elink..
:dt.CTRL-f r             
:dd.replace. For options see :link reftype=hd res=17.Find and replace:elink..
:dt.CTRL-g b             
:dd.goto beginning of marked block
:dt.CTRL-g e             
:dd.goto end of marked block
:dt.CTRL-g g             
:dd.goto marker
:dt.CTRL-g l             
:dd.last position (stored before CTRL-g, jump to beginning
or end of file, searching, replacing)
:dt.CTRL-g n             
:dd.goto line number
:dt.CTRL-g s             
:dd.set marker
:dt.CTRL-j                  
:dd.Join (append following line to the current one)
:dt.CTRL-k                  
:dd.(from the German korrigieren (correct)) insert the
line deleted last (with CTRL-y) before the current line
:dt.CTRL-l                  
:dd.Refresh (VI-like)
:dt.CTRL-n                  
:dd.Repeat find/replace (same as CTRL-f-a)
:dt.CTRL-o                  
:dd.(VI-like open line) insert line after current line
:dt.CTRL-p
:dd.Jumps to the bracket (Parenthesis), that belongs to the one,
being nearest to the cursor.
:dt.CTRL-q                  
:dd.Delete to end-of-line
:dt.CTRL-r                  
:dd.repeat a command sequence n times
:dt.CTRL-s a             
:dd.autoindent
:dt.CTRL-s b             
:dd.blockhighlighting
:dt.CTRL-s c             
:dd.tabcompression (blanks may be converted to tabs when
saving the file, this is the default)
:dt.CTRL-s l
:dd.toggle the automatic linebreak mode
:dt.CTRL-s r             
:dd.regular expressions (like with GREP) for searching
:dt.CTRL-s s             
:dd.shell mode (when pressing RETURN, the part of the
current line up to the cursor position is passed to
the shell, the result of the executed command is
inserted in the text and marked as a block)
:dt.CTRL-s t             
:dd.define tablength (how far the TAB key ought to jump,
not what number of spaces is converted to a tab in
the text's file representation)
:dt.CTRL-s .             
:dd.create .bak-files when overwriting an existing file
:dt.CTRL-t                  
:dd.Delete word right to the cursor (Borland-like)
:dt.CTRL-u                  
:dd.toggle underline mode (underlined characters are
represented by _^H<x> where <x> is the underlined
character).
:dt.CTRL-v                  
:dd.Macro definition/execution
:dt.CTRL-w c             
:dd.close current window
:dt.CTRL-w d             
:dd.switch direct to another window by supplying the
name of the file contained in the window
:dt.CTRL-w i             
:dd.switch to window by index (number of the window
displayed in the lower left corner of each window)
:dt.CTRL-w m             
:dd.move window on the screen
:dt.CTRL-w n             
:dd.switch to next window
:dt.CTRL-w o             
:dd.open new window
:dt.CTRL-w p             
:dd.switch to previous window. This operation is symmetric,
i.e. applied twice, you get to the window where you
were before.
:dt.CTRL-w s             
:dd.change size of current window
:dt.CTRL-w t             
:dd.toggle window size (two sizes exist, both of which
are initially set to full size)
:dt.CTRL-x n             
:dd.exit without saving and without executing the load file
(the load file is a batch file, which can optionally
be executed when quitting the editor, e.g. it may
contain the call to MAKE)
:dt.CTRL-x s             
:dd.exit with saving all modified files, but without
executing the load file
:dt.CTRL-x N             
:dd.exit without saving, but with loadfile execution.
The default name for the load file is load.bat in
the current directory, but it can be modified using
the environment variable EHPINIT.
:dt.CTRL-x S             
:dd.exit with saving all modified files and executing the
load file.
:dt.CTRL-y                  
:dd.delete the current line (can be undone using :hp2.CTRL-k:ehp2.)
:dt.arr_left                
:dd.move cursor left one character
:dt.arr_right               
:dd.move cursor right one character
:dt.arr_up                  
:dd.move cursor up one character
:dt.arr_down                
:dd.move cursor down one character
:dt.CTRL-arr_left           
:dd.move cursor left one word
:dt.CTRL-arr_right          
:dd.move cursor right one word
:dt.CTRL-arr_up             
:dd.move window content including cursor one line up
:dt.CTRL-arr_down           
:dd.move window content including cursor one line down
:dt.ALT-arr_left            
:dd.make current line appear in the middle of the window
:dt.ALT-arr_right           
:dd.move cursor to the middle of the current window
:dt.ALT-arr_up              
:dd.make current line first line of window
:dt.ALT-arr_down            
:dd.make current line last line of window
:dt.HOME                    
:dd.move cursor to beginning of current line
:dt.END                     
:dd.move cursor to end of current line
:dt.CTRL-HOME               
:dd.move cursor to the first line of the window
:dt.CTRL-END                
:dd.move cursor to the last line of the window
:dt.PgUp                    
:dd.one page up
:dt.PgDn                    
:dd.one page down
:dt.CTRL-PgUp               
:dd.go to beginning of file
:dt.CTRL-PgDn               
:dd.go to end of file
:dt.ALT-PgUp                
:dd.go half a page up
:dt.ALT-PgDn                
:dd.go half a page down
:dt.DEL                     
:dd.delete character on which the cursor stands
:dt.Backspace               
:dd.delete character left to the cursor. If cursor is in
first column, join current line with the upper line.
:dt.Ins                     
:dd.Toggle insert/overwrite mode
:edl.

.* *****************************
:h2 res=11. Mouse commands
.* *****************************

:ul.
:li.Left button:
:lp.
:dl compact tsize=52 break=fit.
:dthd.:hp5.Position:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.click on text in active window
:dd.Set the cursor to the current mouse position
:dt.click on text in non-active window
:dd.Activate that window (cursor position is NOT set to mouse position). If
you want to set the cursor to the mouse position in the just activated
window, click the left button again.
:dt.click and move on text
:dd.Mark a normal block (like CTRL-b b and CTRL-b n)
:dt.click on headline and move
:dd.Move the window
:dt.click on lower right corner of window and move
:dd.Resize that window
:dt.click on upper left corner
:dd.Close that window (like CTRL-w c)
:dt.click on upper right corner
:dd.Toggle window size (like CTRL-w t)
:dt.click on arrows in frame
:dd.Scroll text in arrow direction
:dt.click on yes/no-question
:dd.YES
:edl.

:li.Right button:
:lp.
:dl compact tsize=52 break=fit.
:dthd.:hp5.Position:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.click on text with no block marked and no block in PASTE-buffer
:dd.Set the cursor to the current mouse position and activate that window
(if not already activated)
:dt.click on text with no block marked and block in the PASTE-buffer
:dd.Paste the block from the PASTE-buffer into the text at the current
mouse position
:dt.click on text with a block marked in the window clicked to
:dd.The marked block is stored in the PASTE-buffer and unmarked
:dt.click and move on text
:dd.Mark a rectangular block (like CTRL-b b and CTRL-b r)
:dt.click on headline
:dd.Push the window into the background
:dt.click on lower right corner of window and move
:dd.Resize that window
:dt.click on upper left corner
:dd.Close that window (like CTRL-w c)
:dt.click on upper right corner
:dd.Toggle window size (like CTRL-w t)
:dt.click on arrows in frame
:dd.Scroll text in arrow direction
:dt.click on yes/no-question
:dd.NO
:edl.

:li.Middle and left: (press middle first, then left, probably three-button-mouse
is not supported by OS/2 at the moment)
:lp.
:dl compact tsize=52 break=fit.
:dthd.:hp5.Position:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.on text
:dd.move marked block to mouseposition
:edl.

:li.Middle and right: (press middle first, then right, probably 
three-button-mouse is not supported by the mouse driver)
:lp.
:dl compact tsize=52 break=fit.
:dthd.:hp5.Position:ehp5.
:ddhd.:hp5.Effect:ehp5.
:dt.on text or frame with marked block in window clicked to
:dd.delete marked block in current window
:edl.
:eul.

:euserdoc.
