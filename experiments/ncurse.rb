#!/usr/bin/ruby -w

require '../utils'
include UtilityFunctions
require 'ncurses'
include Ncurses

# Testing ncurses functions

Ncurses::initscr
begin
	Ncurses::cbreak
	Ncurses::clear

	Ncurses::mvprintw( 0,0, "Fetching columns and rows." )

	rows, cols = [], []
	Ncurses::stdscr.getmaxyx( rows, cols )

	Ncurses::mvprintw( 2, 0, "Cols: %s" % cols.inspect )
	Ncurses::mvprintw( 3, 0, "Rows: %s" % rows.inspect )

	Ncurses::mvprintw( 5, 0, "COLS: %s" % Ncurses.COLS.inspect )
	Ncurses::mvprintw( 6, 0, "LINES: %s" % Ncurses.LINES.inspect )

	Ncurses::getch
ensure
	Ncurses::curs_set(1)
	Ncurses::endwin()
end
