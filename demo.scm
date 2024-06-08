
(use-modules (stampede))

(define conn (conn-from-params params))
(define query "SELECT * FROM InventoryItem;")

(when (good-conn? conn)
  (display (fetch-all conn query))
  (display (fetch-all conn query)))
