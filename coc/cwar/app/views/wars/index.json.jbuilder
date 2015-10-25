json.array!(@wars) do |war|
  json.extract! war, :id
  json.url war_url(war, format: :json)
end
