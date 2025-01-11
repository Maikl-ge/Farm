def safe_get(data, key, default=0):
    return data.get(key) if data.get(key) is not None else default