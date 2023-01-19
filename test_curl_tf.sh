for x in {1..100}; do
  curl --data-binary @cat.jpg -H 'Content-Type:application/x-image' \
  -X POST http://localhost:8080/invocations
done
