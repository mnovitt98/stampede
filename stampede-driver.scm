
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

;; make macro for "with-conn conn" that stays in the same
;; transaction block for the extenet of the body

(define new-conn (conn-from-params params))
(when (good-conn? new-conn)
  (dump-exec new-conn "SELECT * FROM InventoryItem;"))
