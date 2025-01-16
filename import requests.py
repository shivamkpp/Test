import requests
import time
import hmac
import hashlib

# API credentials
client_id = '98khmWgl'
client_secret = 'TB1_-D1qLrO33sTdiJgTUQIJRXBQyIQIFEkDL7Hd16Q'

# Deribit API endpoint
base_url = 'https://test.deribit.com'

# Authenticate and get access token
def authenticate():
    endpoint = '/api/v2/public/auth'
    params = {
        'grant_type': 'client_credentials',
        'client_id': client_id,
        'client_secret': client_secret
    }
    response = requests.get(base_url + endpoint, params=params)
    data = response.json()
    if 'result' in data:
        return data['result']['access_token']
    else:
        raise Exception('Authentication failed: {}'.format(data))

# Place an order
def place_order(access_token, instrument_name, amount, price, order_type='limit'):
    endpoint = '/api/v2/private/buy'
    headers = {
        'Authorization': f'Bearer {access_token}'
    }
    params = {
        'instrument_name': instrument_name,
        'amount': amount,
        'type': order_type,  # "limit" or "market"
        'price': price  # Required for limit orders
    }
    response = requests.get(base_url + endpoint, headers=headers, params=params)
    data = response.json()
    if 'result' in data:
        return data['result']
    else:
        raise Exception('Order placement failed: {}'.format(data))

# Example usage
try:
    # Step 1: Authenticate
    token = authenticate()
    print("Authenticated successfully. Access token:", token)

    # Step 2: Place an order
    instrument = 'BTC-PERPETUAL'  # Example instrument
    order_amount = 10  # Example order size
    order_price = 40000.0  # Example price for limit order
    order_response = place_order(token, instrument, order_amount, order_price)
    print("Order placed successfully:", order_response)

except Exception as e:
    print(str(e))
