<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Light from '~icons/tabler/bulb';
	import Info from '~icons/tabler/info-circle';
	import Save from '~icons/tabler/device-floppy';
	import Reload from '~icons/tabler/reload';
	import { socket } from '$lib/stores/socket';
	import type { LightState } from '$lib/types/models';
	import GridForm from '$lib/components/GridForm.svelte';
	import Slider from '$lib/components/Slider.svelte';
	import Checkbox from '$lib/components/Checkbox.svelte';
	import { debounce } from 'lodash';

	let lightState: LightState = { led_on: false, red: 0, green: 0, blue: 1 };

	let lightOn = false;

	let rgb_enabled = $page.data.features.rgb;

	async function getLightstate() {
		try {
			const response = await fetch('/rest/lightState', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			const light = await response.json();
			lightOn = light.led_on;
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	onMount(() => {
		socket.on<LightState>('led', (data) => {
			lightState = data;
		});
		getLightstate();
	});

	let setLightState = debounce(() => {
		socket.sendEvent('led', lightState);
	}, 200, {leading:true, trailing:true, maxWait:200});

	onDestroy(() => socket.off('led'));

	async function postLightstate() {
		try {
			const response = await fetch('/rest/lightState', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ led_on: lightOn })
			});
			if (response.status == 200) {
				notifications.success('Light state updated.', 3000);
				const light = await response.json();
				lightOn = light.led_on;
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}
</script>

<SettingsCard collapsible={false}>
	<Light slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Light State</span>
	<div class="w-full">
		<h1 class="text-xl font-semibold">REST Example</h1>
		<div class="alert alert-info my-2 shadow-lg">
			<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
			<span>The form below controls the LED via the RESTful service exposed by the ESP device.</span
			>
		</div>
		<div class="flex flex-row flex-wrap justify-between gap-x-2">
			<div class="form-control w-52">
				<label class="label cursor-pointer">
					<span class="mr-4">Light State?</span>
					<input type="checkbox" bind:checked={lightOn} class="checkbox checkbox-primary" />
				</label>
			</div>
			<div class="flex-grow" />
			<button class="btn btn-primary inline-flex items-center" on:click={postLightstate}
				><Save class="mr-2 h-5 w-5" /><span>Save</span></button
			>
			<button class="btn btn-primary inline-flex items-center" on:click={getLightstate}
				><Reload class="mr-2 h-5 w-5" /><span>Reload</span></button
			>
		</div>
		<div class="divider" />
		<h1 class="text-xl font-semibold">Event Socket Example</h1>
		<div class="alert alert-info my-2 shadow-lg">
			<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
			<span
				>The switch below controls the LED via the event system which uses WebSocket under the hood.
				It will automatically update whenever the LED state changes.</span
			>
		</div>
		<GridForm>
			<Checkbox bind:value={lightState.led_on} label="Light State" onChange={setLightState}></Checkbox>
			{#if $page.data.features.rgb}
				<Slider bind:value={lightState.red} label="Red" onChange={setLightState}></Slider>
				<Slider bind:value={lightState.green} label="Green" onChange={setLightState}></Slider>
				<Slider bind:value={lightState.blue} label="Blue" onChange={setLightState}></Slider>
			{/if}
		</GridForm>
	</div>
</SettingsCard>
