#! /usr/bin/guile \
--no-auto-compile -s
!#

(use-modules (srfi srfi-13)
	     (ice-9 match))

;; does not work with dotted pairs!
(define (sexp->json e)
  (match e
    (() "[]")
    ((? number?) (number->string e))
    ((? symbol?) (string-join (list "\"" (symbol->string e) "\"") ""))
    ((h . t) (string-join (list "["
				(string-join (map sexp->json e) ",")
				"]")
			  ""))))

;(display (sexp->json '(1 (2 3 4) qwe)))

(display (sexp->json (read))) (newline) (quit)