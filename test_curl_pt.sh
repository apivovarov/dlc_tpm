for x in {1..10}; do
  curl -s -d "@test_pt.json" -H 'Content-Type: application/json' \
  -X POST http://localhost:8080/invocations
done
