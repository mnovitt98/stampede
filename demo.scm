
(use-modules (stampede))

(define conn (conn-from-params params))
(define query "SELECT * FROM InventoryItem;")

(when (good-conn? conn)
  (dump-exec conn query))
