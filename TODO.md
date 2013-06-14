
# The List of Things I Should Be Doing (if I weren't so busy making this list) 

- Test: portability
  - Have tested: cygwin, macos, freebsd, ubuntu
  - Need more

- Doc: Make man page for libslax (public API)

- Feature: parser issue warnings if 1.1 features with "version 1.0;"

- Code: functions to parse our arguments/table

- opt: handle empty <xsl:otherwise/> (don't make empty "else { }")

- sdb: Conditional breakpoints

- sdb: up, down, and stack-sensitive "print"

- sdb: "frame" command to display detailed information about a stack frame
  including local variables ("info locals"), context node, etc

- Bug: line numbers are off if file doesn't end in NL

- Bug: foo("\"") needs to use single quotes for the wrapping string
    (&apos;&quot;&apos;).  I don't know of a fix when both types of
    quotes appear within a single string, which is just sad.

- Nit: teach slaxProcTrace() how to keep the trace handle open

- Nit: add command line option for profiler (without entering debugger)

- Check: confirm that UTF-8 encoding functions are working correctly

- Nit: Put _all_ debugger info in statep

- sdb: add "reload data" command (default to "reload script")

- libxml2 bug:
  This line gives a decent error message:
      <xsl:if test="get:bug($x, $in">;
  This line does not:
      <func:result select="get:bug($x, $in">;
  It only says "XPath error : Invalid expression" with no file/line info.

- Nit: should give an error for unterminated string (doesn't seem to work yet)
  Mixed reviews: seems to work great in some instances but I've
  seen it not be triggered; need more testing.

- Nit: slaxwriter needs to care about wrapping lines, say, at
  commas, underscores, operators, etc and reindenting the next line

- Bug: "slaxproc -F" turns:
    <sym xml:space="preserve"> " ";
  into:
    <sym xml:space="preserve"> ;

- Feature: --strip-xmlns to remove namespaces completely from
  the input document