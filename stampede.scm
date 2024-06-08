
;; looks like you need to export all of the "behind the scenes" functions defined
;; at the extension level that you plan to use elsewhere, INCLUDING within this
;; module file
(define-module (stampede)
  #:export (make-connection
            params
            conn-from-params
            good-conn?
            dump-exec
            make-result
            get-nth
            result-length
            fetch-all))

(load-extension "libguile-stampede" "init_stampede")

(define params '(("dbname" . "test_db")
                 ("user"   . "test_user")
                 ("host"   . "192.168.1.150")
                 ("port"   . "5432")))
  
(define (conn-from-params alist)
  (make-connection
   (string-join (map (lambda (x)
                       (let ((param (car x))
                             (value (cdr x)))
                         (string-join (list param value) "=")))
                     alist))))

(define (get-all-tuples res)
  (do ((i 0 (1+ i))
       (acc '()))
      ((= i (result-length res))
       acc)
    (set! acc (cons (get-nth res i) acc))))

(define (fetch-all conn query)
  (get-all-tuples (make-result conn query)))
