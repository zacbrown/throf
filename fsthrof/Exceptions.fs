﻿namespace FSThrof

//module Warnings =


module Errors =
    exception CommentNotClosed of string
    exception MissingParseNodeInExecutionStream of string * string * string
    exception UnexpectedParserNode of string
    exception IntegerExpected of string
    exception NumericExpected of string
    exception InvalidOperation of string
    exception StackUnderflow of string * string

