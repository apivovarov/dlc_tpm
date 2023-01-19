for x in {1..10}; do
  curl -s -d "@test_tf.json" -H 'Content-Type:application/x-image' \
  -X POST http://localhost:8080/invocations
done
