*&---------------------------------------------------------------------*
*& Report ZS_REPOSRC_DOWNLOAD
*&---------------------------------------------------------------------*
*& Purpose: Download compressed source code from table REPOSRC
*& Author : Daniel Berlin
*& Version: 1.0.0
*& License: CC BY 3.0 (http://creativecommons.org/licenses/by/3.0/)
*&---------------------------------------------------------------------*

REPORT zs_reposrc_download.

DATA: v_fnam TYPE rlgrap-filename,  " Local file name
      v_file TYPE string,           " Same, but as a string
      v_xstr TYPE xstring,          " Source (compressed)
      v_xlen TYPE i,                " Length of source
      t_xtab TYPE TABLE OF x255.    " Source plugged into a table

PARAMETERS: report TYPE progname DEFAULT sy-repid           "#EC *
                   MATCHCODE OBJECT progname OBLIGATORY.

START-OF-SELECTION.

  " -- Select local file name
  WHILE v_fnam IS INITIAL.
    CALL FUNCTION 'NAVIGATION_FILENAME_HELP'
      EXPORTING
        mode              = 'S'
      IMPORTING
        selected_filename = v_fnam.
  ENDWHILE.

  v_file = v_fnam.

  " -- Fetch compressed source code
  SELECT SINGLE data INTO v_xstr FROM reposrc
          WHERE progname = report AND r3state = 'A'.

  v_xlen = XSTRLEN( v_xstr ).

  " -- Plug source into a table
  CALL METHOD cl_swf_utl_convert_xstring=>xstring_to_table
    EXPORTING
      i_stream = v_xstr
    IMPORTING
      e_table  = t_xtab
    EXCEPTIONS
      OTHERS   = 1.

  " -- Download to local file
  CALL FUNCTION 'GUI_DOWNLOAD'
    EXPORTING
      filetype     = 'BIN'
      filename     = v_file
      bin_filesize = v_xlen
    TABLES
      data_tab     = t_xtab
    EXCEPTIONS
      OTHERS       = 0.
